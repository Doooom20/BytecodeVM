
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <cctype>
#include <limits>

using namespace std;

enum tokenTypes {
    LITERAL, IDENTIFIER, ASSIGN, MATH, LOGIC,
    CONTROL, MEMORY, STACK, IO, SPECIAL
};

enum tokens {
    VAR, CONST, ADD, SUB, MUL, DIV, AND, OR, XOR, NOT,
    IF, ELSE, WHILE, FOR, BREAK, CONTINUE, RETURN, PRINT,
    NUMBER, NAME, EQUAL, LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE, SEMICOLON, COMMA, END
};

struct token {
    tokenTypes type;
    tokens kind;
    uint32_t line;
    uint32_t column;
    string value;
};

static const unordered_map<string, tokens> keywords = {
    {"var", VAR}, {"let", VAR}, {"const", CONST},
    {"if", IF}, {"else", ELSE}, {"while", WHILE},
    {"for", FOR}, {"break", BREAK},
    {"continue", CONTINUE}, {"return", RETURN},
    {"print", PRINT}
};

static tokenTypes categoryOf(tokens kind) {
    switch (kind) {
        case VAR:
        case CONST: return MEMORY;

        case ADD:
        case SUB:
        case MUL:
        case DIV: return MATH;

        case AND:
        case OR:
        case XOR:
        case NOT: return LOGIC;

        case IF:
        case ELSE:
        case WHILE:
        case FOR:
        case BREAK:
        case CONTINUE:
        case RETURN: return CONTROL;

        case PRINT: return IO;
        case NUMBER: return LITERAL;
        case NAME: return IDENTIFIER;
        case EQUAL: return ASSIGN;

        default: return SPECIAL;
    }
}

static token makeToken(
    tokens kind, const string& value,
    uint32_t line, uint32_t column
) {
    return {categoryOf(kind), kind, line, column, value};
}

vector<token> lex(const string& source) {
    vector<token> result;
    size_t i = 0;
    uint32_t line = 1, column = 1;

    auto advance = [&]() -> char {
        char c = source[i++];
        if (c == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
        return c;
    };

    auto add = [&](tokens kind, const string& value,
                   uint32_t l, uint32_t c) {
        result.push_back(makeToken(kind, value, l, c));
    };

    while (i < source.size()) {
        char c = source[i];

        if (isspace(static_cast<unsigned char>(c))) {
            advance();
            continue;
        }

        if (c == '/' && i + 1 < source.size()
                     && source[i + 1] == '/') {
            while (i < source.size() && source[i] != '\n')
                advance();
            continue;
        }

        uint32_t startLine = line;
        uint32_t startColumn = column;

        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t start = i;

            while (i < source.size()) {
                char ch = source[i];
                if (!isalnum(static_cast<unsigned char>(ch))
                    && ch != '_')
                    break;
                advance();
            }

            string word = source.substr(start, i - start);
            auto it = keywords.find(word);

            add(it != keywords.end() ? it->second : NAME,
                word, startLine, startColumn);
            continue;
        }

        if (isdigit(static_cast<unsigned char>(c))) {
            size_t start = i;

            while (i < source.size() &&
                   isdigit(static_cast<unsigned char>(source[i]))) {
                advance();
            }

            add(NUMBER, source.substr(start, i - start),
                startLine, startColumn);
            continue;
        }

        if (i + 1 < source.size()) {
            string op = source.substr(i, 2);

            if (op == "&&" || op == "||") {
                advance();
                advance();
                add(op == "&&" ? AND : OR, op,
                    startLine, startColumn);
                continue;
            }

            if (op == "==") {
                throw runtime_error(
                    "The == operator is not supported yet"
                );
            }
        }

        tokens kind;

        switch (c) {
            case '+': kind = ADD; break;
            case '-': kind = SUB; break;
            case '*': kind = MUL; break;
            case '/': kind = DIV; break;
            case '^': kind = XOR; break;
            case '!': kind = NOT; break;
            case '=': kind = EQUAL; break;
            case '(': kind = LEFT_PAREN; break;
            case ')': kind = RIGHT_PAREN; break;
            case '{': kind = LEFT_BRACE; break;
            case '}': kind = RIGHT_BRACE; break;
            case ';': kind = SEMICOLON; break;
            case ',': kind = COMMA; break;

            default:
                throw runtime_error(
                    "Unexpected character '" + string(1, c) +
                    "' at line " + to_string(line) +
                    ", column " + to_string(column)
                );
        }

        string value(1, advance());
        add(kind, value, startLine, startColumn);
    }

    result.push_back({SPECIAL, END, line, column, ""});
    return result;
}

class Compiler {
private:
    const vector<token>& inputTokens;
    size_t pos = 0;
    unsigned nextRegister = 0;
    unordered_map<string, uint8_t> variables;
    vector<string> assembly;

    const token& peek() const {
        return inputTokens[pos];
    }

    const token& previous() const {
        return inputTokens[pos - 1];
    }

    token consume() {
        return inputTokens[pos++];
    }

    bool match(tokens kind) {
        if (peek().kind != kind)
            return false;
        consume();
        return true;
    }

    token expect(tokens kind, const string& message) {
        if (peek().kind != kind) {
            throw runtime_error(
                "Line " + to_string(peek().line) +
                ", column " + to_string(peek().column) +
                ": " + message +
                " (found '" + peek().value + "')"
            );
        }
        return consume();
    }

    uint8_t allocateRegister() {
        if (nextRegister >= 256)
            throw runtime_error("Out of VM registers");

        return static_cast<uint8_t>(nextRegister++);
    }

    void emit(const string& instruction) {
        assembly.push_back(instruction);
    }

    int precedence(tokens kind) const {
        switch (kind) {
            case ADD:
            case SUB: return 1;

            case MUL:
            case DIV: return 2;

            default: return 0;
        }
    }

    uint8_t parsePrimary() {
        if (match(NUMBER)) {
            string text = previous().value;
            unsigned long long value = stoull(text);

            if (value > numeric_limits<uint32_t>::max())
                throw runtime_error("Integer literal is too large");

            uint8_t reg = allocateRegister();

            // IMM: byte_count, destination_register, value
            emit("IMM 4 " + to_string(reg) + " " +
                 to_string(value));

            return reg;
        }

        if (match(NAME)) {
            string name = previous().value;
            auto it = variables.find(name);

            if (it == variables.end()) {
                throw runtime_error(
                    "Undefined variable '" + name + "'"
                );
            }

            return it->second;
        }

        if (match(LEFT_PAREN)) {
            uint8_t reg = parseExpression();
            expect(RIGHT_PAREN, "Expected ')' after expression");
            return reg;
        }

        throw runtime_error(
            "Expected a number, variable, or parenthesized expression "
            "at line " + to_string(peek().line) +
            ", column " + to_string(peek().column)
        );
    }

    uint8_t parseExpression(int minPrecedence = 1) {
        uint8_t left = parsePrimary();

        while (precedence(peek().kind) >= minPrecedence) {
            tokens op = consume().kind;
            int p = precedence(op);

            uint8_t right = parseExpression(p + 1);
            uint8_t destination = allocateRegister();

            string instruction;

            switch (op) {
                case ADD: instruction = "ADD"; break;
                case SUB: instruction = "SUB"; break;
                case MUL: instruction = "MUL"; break;
                case DIV: instruction = "DIV"; break;
                default:
                    throw runtime_error("Unsupported operator");
            }

            // VM arithmetic operands: source1 source2 destination
            emit(instruction + " " + to_string(left) + " " +
                 to_string(right) + " " + to_string(destination));

            left = destination;
        }

        return left;
    }

    void parseDeclaration() {
        bool isConst = match(CONST);

        if (!isConst)
            expect(VAR, "Expected a variable declaration");

        token name = expect(NAME, "Expected a variable name");

        if (variables.count(name.value)) {
            throw runtime_error(
                "Variable '" + name.value + "' is already declared"
            );
        }

        expect(EQUAL, "Expected '=' after variable name");

        uint8_t valueRegister = parseExpression();

        expect(SEMICOLON, "Expected ';' after declaration");

        // This first version associates the variable with the
        // register containing its initial value.
        variables[name.value] = valueRegister;
    }

    void parsePrint() {
        expect(PRINT, "Expected 'print'");

        uint8_t reg = parseExpression();

        expect(SEMICOLON, "Expected ';' after print statement");

        emit("PRINT " + to_string(reg));
    }

public:
    explicit Compiler(const vector<token>& input) : inputTokens(input) {}

    string compile() {
        while (peek().kind != END) {
            if (peek().kind == VAR || peek().kind == CONST) {
                parseDeclaration();
            } else if (peek().kind == PRINT) {
                parsePrint();
            } else {
                throw runtime_error(
                    "Unexpected token '" + peek().value +
                    "' at line " + to_string(peek().line) +
                    ", column " + to_string(peek().column)
                );
            }
        }

        emit("HALT");

        string output;
        for (const string& instruction : assembly)
            output += instruction + "\n";

        return output;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0]
             << " <input_file> <output.asm>\n";
        return EXIT_FAILURE;
    }

    ifstream inputFile(argv[1]);

    if (!inputFile) {
        cerr << "Error opening input file: " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    string source{istreambuf_iterator<char>(inputFile), istreambuf_iterator<char>()};

    try {
        vector<token> result = lex(source);
        Compiler compiler(result);
        string output = compiler.compile();

        ofstream outputFile(argv[2]);

        if (!outputFile) {
            cerr << "Error opening output file: " << argv[2] << '\n';
            return EXIT_FAILURE;
        }

        outputFile << output;

        if (!outputFile) {
            cerr << "Error writing output file: " << argv[2] << '\n';
            return EXIT_FAILURE;
        }

        cout << "Compilation successful: " << argv[2] << '\n';

    } catch (const exception& e) {
        cerr << "Compiler error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
