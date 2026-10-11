# Custom Assembly CPU

A small C++ virtual CPU and assembler that reads an assembly source file, converts instructions into bytecode, and executes them.

## Requirements

- A C++17-compatible compiler, such as GCC, Clang, or MSVC.
- An assembly source file with the `.asm` extension.

The program uses the C++17 `<filesystem>` library.

## Compilation

Save the C++ source code as `main.cpp`.

```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o asm_cpu
```

Run the program with an assembly file:

```bash
./asm_cpu example.asm
```

## Supported Instructions

| Instruction | Operands | Description |
|---|---|---|
| `ADD` | `src1 src2 dest` | Addition |
| `SUB` | `src1 src2 dest` | Subtraction |
| `MUL` | `src1 src2 dest` | Multiplication |
| `DIV` | `src1 src2 dest` | Integer division |
| `AND` / `OR` / `XOR` | `src1 src2 dest` | Bitwise operations |
| `NOT` | `src dest` | Bitwise complement |
| `IMM` | `bytes dest value` | Load an immediate value |
| `LOAD` | `address_reg dest` | Load from memory |
| `STORE` | `address_reg value_reg` | Store to memory |
| `JMP` | `address_reg` | Unconditional jump |
| `JZ` / `JNZ` | `address_reg test_reg` | Jump if zero / nonzero |
| `JF` | `address_reg test_reg` | Jump if the CPU flag is set |
| `LSL` / `LSR` / `ASR` | `src1 src2 dest` | Bit shifts |
| `LT` / `GT` | `src1 src2 dest` | Comparison instructions |
| `PUSH` | `src` | Push a register value onto the stack |
| `POP` | `dest` | Pop into a register |
| `DROP` | None | Discard the top stack item |
| `DUP` | None | Duplicate the top stack item |
| `SWAP` | None | Swap the top two stack items |
| `PRINT` | `reg` | Print a register value |
| `NOP` | None | Do nothing |
| `HALT` | None | Stop execution |

## Syntax Notes

- Registers are numbered `0` through `255`.
- `IMM` accepts a byte count from `1` to `4`. Values are encoded in little-endian byte order.
- Numeric operands support decimal and C-style prefixes, such as `10`, `0x10`, and `010`.
- Jump instructions use register values as **byte offsets into the assembled program**, not source-line numbers or instruction indices.

## Error Handling

The assembler reports errors for unknown instructions, invalid numbers, incorrect operand counts, and unreadable source files.

The CPU also reports execution errors such as unknown opcodes or unexpected end-of-program conditions.

