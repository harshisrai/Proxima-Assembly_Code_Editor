0x0: auipc x11 65536
0x4: addi x11 x11 18
0x8: add x1 x2 x3
0xc: addi x2 x0 1
0x10: add x3 x2 x2
0x14: or x4 x2 x3
0x18: sll x5 x3 x2
0x1c: slt x5 x3 x6
0x20: sra x2 x3 x4
0x24: srl x2 x3 x4
0x28: sub x4 x5 x6
0x2c: xor x31 x6 x1
0x30: mul x8 x31 x4
0x34: div x30 x8 x7
0x38: rem x29 x30 x31
0x3c: andi x28 x29 -100
0x40: ori x27 x5 69
0x44: jal x15 branch1
0x48: addi x16 x0 1
0x4c: auipc x16 65536
0x50: sb x8 0 x16
0x54: sw x5 6 x16
0x58: sh x30 -40 x16
0x5c: beq x5 x6 branch1
0x60: bne x5 x6 branch2
0x64: bge x8 x9 branch3
0x68: blt x13 x14 branch4
0x6c: lui x6  10000
0x70: jalr x1 x15 0

