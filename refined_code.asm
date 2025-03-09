<<<<<<< HEAD
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
=======
0x0: addi t3  x0  0x100
0x4: slli t3  t3  20
0x8: lw a1  0  t3
0xc: lw a2  4  t3
0x10: addi t0  x0  0
0x14: addi t1  x0  1
0x18: sw t0  0(a2)
0x1c: addi a1  a1  -1
0x20: addi a2  a2  4
0x24: sw t1  0(a2)
0x28: addi a1  a1  -1
0x2c: add t2  t1  t0
0x30: addi a2  a2  4
0x34: sw t2  0(a2)
0x38: addi a1  a1  -1
0x3c: jal x0 keep_looping
>>>>>>> 53d2ded58d1b4380fb0315c83866459bb7af998c

