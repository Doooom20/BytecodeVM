#include <cstdint>
#include <iostream>

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
    LSL,
    LSR,
    ASR,
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

void execute_instruction(uint32_t instruction, cpu& targetCPU){
    targetCPU.opcode = (instruction >> 24) & 0xFF;
    targetCPU.source1 = (instruction >> 16) & 0xFF;
    targetCPU.source2 = (instruction >> 8) & 0xFF;
    targetCPU.destination = instruction & 0xFF;
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
            targetCPU.pc = targetCPU.registers[targetCPU.source1] - 1;
            break;
        case JZ:
            if(targetCPU.registers[targetCPU.source2] == 0){
                targetCPU.pc = targetCPU.registers[targetCPU.source1] - 1;
            }
            break;
        case JNZ:
            if(targetCPU.registers[targetCPU.source2] != 0){
                targetCPU.pc = targetCPU.registers[targetCPU.source1] - 1;
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
        case IMM:
            targetCPU.registers[targetCPU.destination] = targetCPU.source1 + (targetCPU.source2 * 256);
            break;
        case PRINT:
            std::cout << targetCPU.registers[targetCPU.source1];
            std::cout << std::endl;
            break;
        case HALT:
            std::cout << "HALT!";
            std::cout << std::endl;
            exit(0);
            break;
        case ERR:
            std::cout << "ERROR!";
            break;
    }
}

uint8_t instructions[65536] = {IMM, 42, 0, 0, IMM, 3, 0, 1, MUL, 0, 1, 0, PRINT, 0, 0, 0, HALT, 0, 0, 0};

uint32_t assemble_instruction(uint32_t pointer) {
    uint8_t opcode = instructions[pointer];
    uint8_t source1 = instructions[pointer + 1];
    uint8_t source2 = instructions[pointer + 2];
    uint8_t destination = instructions[pointer + 3];
    return (opcode << 24) | (source1 << 16) | (source2 << 8) | destination;
}

const uint32_t assembledInsructionSize = 65526;

int main() {
    cpu mycpu{};
    uint32_t pc = 0;
    uint32_t assembled_instructions[assembledInsructionSize];
    for (uint32_t i = 0; i < assembledInsructionSize; i += 4) {
        assembled_instructions[i/4] = assemble_instruction(i);
    }
    std::cout << "Assembly Finished!" << std::endl;
    std::cout << "Starting CPU execution..." << std::endl;

    for(mycpu.pc = 0; mycpu.pc < assembledInsructionSize; mycpu.pc += 1) {
        execute_instruction(assembled_instructions[mycpu.pc], mycpu);
    }

    return 0;
}