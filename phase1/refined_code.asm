0x0: add x1 x2 x3
0x4: addi x2 x0 1
0x8: add x3 x2 x2
0xc: or x4 x2 x3
0x10: sll x5 x3 x2
0x14: slt x5 x3 x6
0x18: sra x2 x3 x4
0x1c: srl x2 x3 x4
0x20: sub x4 x5 x6
0x24: xor x31 x6 x1
0x28: mul x8 x31 x4
0x2c: div x30 x8 x7
0x30: rem x29 x30 x31
0x34: andi x28 x29 -100
0x38: ori x27 x5 69
0x3c: jal x15 branch1
0x40: addi x16 x0 1
0x44: auipc x16 65536
0x48: sb x8 0 x16
0x4c: sw x5 6 x16
0x50: sh x30 -40 x16
0x54: beq x5 x6 branch1
0x58: bne x5 x6 branch2
0x5c: bge x8 x9 branch3
0x60: blt x13 x14 branch4
0x64: lui x6  10000
0x68: jalr x1 x15 0

