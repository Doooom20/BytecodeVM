#include <cstdint>      // uint8_t
#include <sstream>      // istringstream
#include <string>       // string
#include <stdexcept>    // runtime_error
#include <cstdlib>      // EXIT_FAILURE, EXIT_SUCCESS
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace std;

vector<uint8_t> emit(uint8_t opcode, const vector<uint8_t>& operands) {
    vector<uint8_t> instruction;
    instruction.push_back(opcode);
    instruction.insert(instruction.end(), operands.begin(), operands.end());
    return instruction;
}

enum operandType {
    IMMEDIATE,
    VARIABLE,
    FUNCTION,
    LABEL,
    EXPRESSION,
    BOOLEAN,
    NONE,
};

enum keyWords {
    LET,
    IF,
    ELSE,
    WHILE,
    FOR,
    RETURN,
    BREAK,
    CONTINUE,
    DEF,
    CONST,
    TRUE,
    FALSE,
    NULL_KEYWORD,
};

struct keyword {
    keyWords type;
    std::string string;
    operandType inputType;
    operandType outputType;
};

keyword keywords[] = {
    {LET, "let", IMMEDIATE, VARIABLE},
    {IF, "if", BOOLEAN, NONE},
    {ELSE, "else", NONE, NONE},
    {WHILE, "while", BOOLEAN, NONE},
    {FOR, "for", BOOLEAN, NONE},
    {RETURN, "return", VARIABLE, NONE},
    {BREAK, "break", NONE, NONE},
    {CONTINUE, "continue", NONE, NONE},
    {DEF, "def", FUNCTION, NONE},
    {CONST, "const", IMMEDIATE, VARIABLE},
    {TRUE, "true", NONE, BOOLEAN},
    {FALSE, "false", NONE, BOOLEAN},
    {NULL_KEYWORD, "null", NONE, NONE}
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << endl;
        return EXIT_FAILURE;
    }

    string inputFilePath = argv[1];
    string outputFilePath = argv[2];
    ifstream inputFile(inputFilePath, ios::binary);
    if (!inputFile) {
        cerr << "Error opening file: " << inputFilePath << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
