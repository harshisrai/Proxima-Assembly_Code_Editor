# RISC-V Assembler - Phase 1: Conversion of Assembly Code to Machine Code

## Overview
This project involves writing a **32-bit RISC-V assembler** in **C++**. The assembler takes a RISC-V assembly code file (`input.asm`) as input and generates a corresponding machine code file (`output.mc`).

## Input and Output Format
### Input (`input.asm`)
The input assembly file will contain one instruction per line in the following format:
```assembly
add x1, x2, x3
andi x5, x6, 10
...
```

### Output (`output.mc`)
The output machine code file will follow this format:
```plaintext
<address> <machine code> , <assembly instruction> # <opcode-func3-func7-rd-rs1-rs2-immediate>
```
#### Example:
```plaintext
0x0 0x003100B3 , add x1,x2,x3 # 0110011-000-0000000-00001-00010-00011-NULL
0x4 0x00A37293 , andi x5,x6,10 # 0010011-111-NULL-00101-00110-000000001010
0x8 <termination code>
```
Additionally, the `.mc` file must also contain the data segment:
```plaintext
0x10000000 0x10
...
```

## Assumptions
- **Code segment** starts at `0x00000000`.
- **Data segment** starts at `0x10000000`.
- **Heap** starts at `0x10008000`.
- **Stack segment** starts at `0x7FFFFFFC`.
- The assembler handles **labels** appropriately.
- Doesn't support **floating-point operations** yet.
- **Pseudo-instruction support will come soon**.

## Supported Instructions
The assembler supports the following **31 RISC-V 32-bit instructions**:

### R-Format:
- `add`, `and`, `or`, `sll`, `slt`, `sra`, `srl`, `sub`, `xor`, `mul`, `div`, `rem`

### I-Format:
- `addi`, `andi`, `ori`, `lb`, `ld`, `lh`, `lw`, `jalr`

### S-Format:
- `sb`, `sw`, `sd`, `sh`

### SB-Format:
- `beq`, `bne`, `bge`, `blt`

### U-Format:
- `auipc`, `lui`

### UJ-Format:
- `jal`

## Supported Assembler Directives
- `.text`
- `.data`
- `.byte`, `.half`, `.word`, `.dword`
- `.asciz`


