#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>

using namespace std;

std::filesystem::path filePath = "./main.asm";
std::vector<uint8_t> instructions;

struct cpu {
    uint32_t registers[256];
    uint32_t pc;
    uint32_t stack[1024];
    uint32_t sp;
    uint32_t memory[65536];
    uint8_t opcode;
    uint8_t source1;
    uint8_t source2;
    uint8_t destination;
    bool halted;
    bool flag;
};

enum opcodes:uint8_t {
    ADD,
    SUB,
    MUL,
    DIV,
    AND,
    OR,
    XOR,
    NOT,
    LOAD,
    STORE,
    JMP,
    JZ,
    JNZ,
    JF,
    LSL,
    LSR,
    ASR,
    LT,
    GT,
    NOP,
    POP,
    PUSH,
    DROP,
    DUP,
    SWAP,
    IMM,
    PRINT,
    HALT,
    ERR = 255
};

struct assembler {
    uint8_t opcode;
    std::string string;
    uint8_t operands;
};

assembler assembler_instructions[] = {
    {ADD, "ADD", 3},
    {SUB, "SUB", 3},
    {MUL, "MUL", 3},
    {DIV, "DIV", 3},
    {AND, "AND", 3},
    {OR, "OR", 3},
    {XOR, "XOR", 3},
    {NOT, "NOT", 2},
    {LOAD, "LOAD", 2},
    {STORE, "STORE", 2},
    {JMP, "JMP", 1},
    {JZ, "JZ", 2},
    {JNZ, "JNZ", 2},
    {JF, "JF", 1},
    {LT, "LT", 3},
    {GT, "GT", 3},
    {LSL, "LSL", 3},
    {LSR, "LSR", 3},
    {ASR, "ASR", 3},
    {NOP, "NOP", 0},
    {POP, "POP", 1},
    {PUSH, "PUSH", 1},
    {DROP, "DROP", 0},
    {DUP, "DUP", 0},
    {SWAP, "SWAP", 0},
    {IMM, "IMM", 3},
    {PRINT, "PRINT", 1},
    {HALT, "HALT", 0}
};

uint8_t fetch_byte(const std::vector<uint8_t>& program, cpu& targetCPU
) {
    if (targetCPU.pc >= program.size()) {
        throw std::runtime_error("Unexpected end of program");
    }

    return program[targetCPU.pc++];
}

void execute_instruction(const std::vector<uint8_t>& program, cpu& targetCPU) {
    targetCPU.opcode = fetch_byte(program, targetCPU);

    auto reg = [&]() -> uint8_t {
        return fetch_byte(program, targetCPU);
    };
    switch (targetCPU.opcode) {
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case AND:
        case OR:
        case XOR:
        case LSL:
        case LSR:
        case ASR:
            targetCPU.source1 = reg();
            targetCPU.source2 = reg();
            targetCPU.destination = reg();
            break;

        case NOT:
        case LOAD:
            targetCPU.source1 = reg();
            targetCPU.destination = reg();
            break;

        case STORE:
            targetCPU.source1 = reg();
            targetCPU.destination = reg();
            break;

        case JMP:
        case PUSH:
        case PRINT:
            targetCPU.source1 = reg();
            break;

        case JZ:
        case JNZ:
            targetCPU.source1 = reg();
            targetCPU.source2 = reg();
            break;

        case JF:
            targetCPU.source1 = reg();
            targetCPU.source2 = reg();
            break;

        case POP:
            targetCPU.destination = reg();
            break;

        case NOP:
        case DROP:
        case DUP:
        case SWAP:
        case HALT:
            break;

        case IMM: {
            uint8_t byteCount = reg();
            uint8_t destination = reg();

            if (byteCount < 1 || byteCount > 4) {
                throw std::runtime_error("Invalid IMM byte count");
            }

            uint32_t value = 0;

            for (uint8_t i = 0; i < byteCount; ++i) {
                value |= uint32_t(fetch_byte(program, targetCPU))
                         << (8 * i);
            }

            targetCPU.registers[destination] = value;
            return;
        }

        default:
            throw std::runtime_error("Unknown opcode");
    }

    switch(targetCPU.opcode){
        case ADD:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] + targetCPU.registers[targetCPU.source2];
            break;
        case SUB:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] - targetCPU.registers[targetCPU.source2];
            break;
        case MUL:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] * targetCPU.registers[targetCPU.source2];
            break;
        case DIV:
            if(targetCPU.registers[targetCPU.source2] != 0){
                targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] / targetCPU.registers[targetCPU.source2];
            } else {
                std::cout << "ERROR";
            }
            break;
        case AND:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] & targetCPU.registers[targetCPU.source2];
            break;
        case OR:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] | targetCPU.registers[targetCPU.source2];
            break;
        case XOR:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] ^ targetCPU.registers[targetCPU.source2];
            break;
        case NOT:
            targetCPU.registers[targetCPU.destination] = ~targetCPU.registers[targetCPU.source1];
            break;
        case LOAD:
            targetCPU.registers[targetCPU.destination] = targetCPU.memory[targetCPU.registers[targetCPU.source1]];
            break;
        case STORE:
            targetCPU.memory[targetCPU.registers[targetCPU.source1]] = targetCPU.registers[targetCPU.destination];
            break;
        case JMP:
            targetCPU.pc = targetCPU.registers[targetCPU.source1];
            break;
        case JZ:
            if(targetCPU.registers[targetCPU.source2] == 0){
                targetCPU.pc = targetCPU.registers[targetCPU.source1];
            }
            break;
        case JNZ:
            if(targetCPU.registers[targetCPU.source2] != 0){
                targetCPU.pc = targetCPU.registers[targetCPU.source1];
            }
            break;
        case JF:
            if(targetCPU.flag){
                targetCPU.pc = targetCPU.registers[targetCPU.source1];
            }
            break;
        case LSL:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] << targetCPU.registers[targetCPU.source2];
            break;
        case LSR:
            targetCPU.registers[targetCPU.destination] = targetCPU.registers[targetCPU.source1] >> targetCPU.registers[targetCPU.source2];
            break;
        case ASR:
            targetCPU.registers[targetCPU.destination] = static_cast<int32_t>(targetCPU.registers[targetCPU.source1]) >> targetCPU.registers[targetCPU.source2];
            break;
        case LT:
            targetCPU.registers[targetCPU.flag] = targetCPU.registers[targetCPU.source1] < targetCPU.registers[targetCPU.source2];
            break;
        case GT:
            targetCPU.registers[targetCPU.flag] = targetCPU.registers[targetCPU.source1] > targetCPU.registers[targetCPU.source2];
            break;
        case NOP:
            break;
        case POP:
            if(targetCPU.sp > 0){
                targetCPU.sp--;
                targetCPU.registers[targetCPU.destination] = targetCPU.stack[targetCPU.sp];
            }
            break;
        case PUSH:
            if(targetCPU.sp < 1024){
                targetCPU.stack[targetCPU.sp] = targetCPU.registers[targetCPU.source1];
                targetCPU.sp++;
            }
            break;
        case DROP:
            if(targetCPU.sp > 0){
                targetCPU.sp--;
            }
            break;
        case DUP:
            if(targetCPU.sp > 0 && targetCPU.sp < 1023){
                targetCPU.stack[targetCPU.sp] = targetCPU.stack[targetCPU.sp - 1];
                targetCPU.sp++;
            }
            break;
        case SWAP:
            if(targetCPU.sp > 1){
                uint32_t temp = targetCPU.stack[targetCPU.sp - 1];
                targetCPU.stack[targetCPU.sp - 1] = targetCPU.stack[targetCPU.sp - 2];
                targetCPU.stack[targetCPU.sp - 2] = temp;
            }
            break;
        case PRINT:
            std::cout << targetCPU.registers[targetCPU.source1];
            std::cout << std::endl;
            break;
        case HALT:
            std::cout << "HALT!";
            std::cout << std::endl;
            targetCPU.halted = true;
            break;
        case ERR:
            std::cout << "ERROR!";
            break;
        default:
            std::cout << "Unknown opcode: " << static_cast<int>(targetCPU.opcode) << std::endl;
            break;
    }
}

uint32_t parse_number(const std::string& text, uint32_t maxValue) {
    if (text.empty() || text[0] == '-') {
        throw std::runtime_error("Invalid unsigned number: " + text);
    }

    size_t parsed = 0;
    unsigned long long value = std::stoull(text, &parsed, 0);

    if (parsed != text.size() || value > maxValue) {
        throw std::runtime_error("Number out of range: " + text);
    }

    return static_cast<uint32_t>(value);
}

uint8_t parse_operand(const std::string& text) {
    return static_cast<uint8_t>(parse_number(text, 255));
}

void assemble_instruction() {
    std::ifstream filein(filePath);

    if (!filein.is_open()) {
        throw std::runtime_error(
            "Could not open assembly file: " + filePath.string()
        );
    }

    instructions.clear();

    std::string line;
    uint32_t lineNumber = 0;

    while (std::getline(filein, line)) {
        ++lineNumber;

        // Remove comments.
        size_t comment = line.find(';');
        if (comment != std::string::npos) {
            line.erase(comment);
        }

        std::istringstream stream(line);
        std::string mnemonic;

        if (!(stream >> mnemonic)) {
            continue;
        }

        // Find the instruction definition.
        const assembler* definition = nullptr;

        for (const auto& entry : assembler_instructions) {
            if (mnemonic == entry.string) {
                definition = &entry;
                break;
            }
        }

        if (definition == nullptr) {
            throw std::runtime_error(
                "Unknown instruction on line " +
                std::to_string(lineNumber) + ": " + mnemonic
            );
        }

        // Read operands, rejecting more than three.
        std::vector<std::string> operands;
        std::string token;

        while (stream >> token) {
            operands.push_back(token);

            if (operands.size() > 3) {
                throw std::runtime_error(
                    "Too many operands on line " +
                    std::to_string(lineNumber)
                );
            }
        }

        try {
            const uint8_t opcode = definition->opcode;

            if (opcode == IMM) {
                // Syntax: IMM byte_count destination value
                if (operands.size() != 3) {
                    throw std::runtime_error(
                        "IMM expects: IMM byte_count destination value"
                    );
                }

                uint32_t byteCount =
                    parse_number(operands[0], 4);

                if (byteCount < 1) {
                    throw std::runtime_error(
                        "IMM byte_count must be between 1 and 4"
                    );
                }

                uint8_t destination = parse_operand(operands[1]);

                uint32_t maxValue;

                if (byteCount == 4) {
                    maxValue = UINT32_MAX;
                } else {
                    maxValue = (uint32_t(1) << (byteCount * 8)) - 1;
                }

                uint32_t value =
                    parse_number(operands[2], maxValue);

                // Emit opcode, size, and destination.
                instructions.push_back(opcode);
                instructions.push_back(
                    static_cast<uint8_t>(byteCount)
                );
                instructions.push_back(destination);

                // Emit immediate value in little-endian order.
                for (uint32_t i = 0; i < byteCount; ++i) {
                    instructions.push_back(
                        static_cast<uint8_t>(
                            (value >> (8 * i)) & 0xFF
                        )
                    );
                }
            } else {
                // Other instructions: opcode followed by operands.
                if (operands.size() != definition->operands) {
                    throw std::runtime_error(
                        mnemonic + " expects " + std::to_string(definition->operands) + " operand(s), but received " + std::to_string(operands.size())
                    );
                }

                instructions.push_back(opcode);

                for (const std::string& operand : operands) {
                    instructions.push_back(parse_operand(operand));
                }
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "Line " + std::to_string(lineNumber) +
                ": " + e.what()
            );
        }
    }

    if (filein.bad()) {
        throw std::runtime_error("Error reading assembly file");
    }

    if (instructions.empty()) {
        throw std::runtime_error("Assembly file contains no instructions");
    }
}



int main(int argc, char* argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " [file.asm]\n";
            return 1;
        }

        if (argc == 2) {
            filePath = argv[1];
        }

        assemble_instruction();

        std::cout << "Assembly finished! "
                  << instructions.size()
                  << " bytes assembled.\n";

        cpu mycpu{};

        std::cout << "Starting CPU execution...\n";

        while (!mycpu.halted && mycpu.pc < instructions.size()) {
            execute_instruction(instructions, mycpu);
        }

    } catch (const std::exception& e) {
        std::cerr << "Assembler/CPU error: "
                  << e.what() << '\n';
        return 1;
    }

    return 0;
}