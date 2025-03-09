0x00000000: add  x1, x2, x3
0x00000004: sub  x4, x5, x6
0x00000008: sll  x7, x8, x9
0x0000000c: slt  x10, x11, x12
0x00000010: sltu x13, x14, x15
0x00000014: xor  x16, x17, x18
0x00000018: srl  x19, x20, x21
0x0000001c: sra  x22, x23, x24
0x00000020: or   x25, x26, x27
0x00000024: and  x28, x29, x30
0x00000028: addi x1, x1, 0b11
0x0000002c: addi x2, x2, 2047
0x00000030: xori x3, x3, 0xFFF
0x00000034: slli x4, x4, 31
0x00000038: srli x5, x5, 31
0x0000003c: srai x6, x6, 31
0x00000040: lw   x7, 1023(x8)
0x00000044: lb   x9, -128(x10)
0x00000048: lhu  x11, 255(x12)
0x0000004c: sw   x1, -1024(x2)
0x00000050: sb   x3, 127(x4)
0x00000054: beq  x1, x2, label_forward
0x00000058: bne  x3, x4, label_backward
0x0000005c: blt  x5, x6, label_far
0x00000060: bge  x7, x8, label_forward
0x00000064: lui  x9, 0xFFFFF
0x00000068: auipc x10, 0x7FFFF
0x0000006c: jal  x1, label_forward
0x00000070: jalr x2, x3, -4
0x00000074: j label_backward
0x00000078: add x1, x2, x3
0x0000007c: j label_forward

