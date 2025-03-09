0x0: add x0, x0, x0
0x4: add  x1, x2, x3
0x8: sub  x4, x5, x6
0xc: sll  x7, x8, x9
0x10: slt  x10, x11, x12
0x14: sltu x13, x14, x15
0x18: xor  x16, x17, x18
0x1c: srl  x19, x20, x21
0x20: sra  x22, x23, x24
0x24: or   x25, x26, x27
0x28: and  x28, x29, x30
0x2c: addi x1, x1, 0b11
0x30: addi x2, x2, 2047
0x34: lw   x7, 1023(x8)
0x38: lb   x9, -128(x10)
0x3c: sw   x1, -1024(x2)
0x40: sb   x3, 127(x4)
0x44: beq  x1, x2, label_forward
0x48: bne  x3, x4, label_backward
0x4c: blt  x5, x6, label_far
0x50: bge  x7, x8, label_forward
0x54: lui  x9, 0xFFFFF
0x58: auipc x10, 0x7FFFF
0x5c: jal  x1, label_forward
0x60: jalr x2, x3, -4
0x64: add x0, x0, x0
0x68: add  x1, x2, x3
0x6c: sub  x4, x5, x6
0x70: sll  x7, x8, x9
0x74: slt  x10, x11, x12
0x78: sltu x13, x14, x15
0x7c: xor  x16, x17, x18
0x80: srl  x19, x20, x21
0x84: sra  x22, x23, x24
0x88: or   x25, x26, x27
0x8c: and  x28, x29, x30
0x90: addi x1, x1, 0b11
0x94: addi x2, x2, 2047
0x98: lw   x7, 1023(x8)
0x9c: lb   x9, -128(x10)
0xa0: sw   x1, -1024(x2)
0xa4: sb   x3, 127(x4)
0xa8: beq  x1, x2, label_forward
0xac: bne  x3, x4, label_backward
0xb0: blt  x5, x6, label_far
0xb4: bge  x7, x8, label_forward
0xb8: lui  x9, 0xFFFFF
0xbc: auipc x10, 0x7FFFF
0xc0: jal  x1, label_forward
0xc4: jalr x2, x3, -4
0xc8: add x0, x0, x0
0xcc: add  x1, x2, x3
0xd0: sub  x4, x5, x6
0xd4: sll  x7, x8, x9
0xd8: slt  x10, x11, x12
0xdc: sltu x13, x14, x15
0xe0: xor  x16, x17, x18
0xe4: srl  x19, x20, x21
0xe8: sra  x22, x23, x24
0xec: or   x25, x26, x27
0xf0: and  x28, x29, x30
0xf4: addi x1, x1, 0b11
0xf8: addi x2, x2, 2047
0xfc: lw   x7, 1023(x8)
0x100: lb   x9, -128(x10)
0x104: sw   x1, -1024(x2)
0x108: sb   x3, 127(x4)
0x10c: beq  x1, x2, label_forward
0x110: bne  x3, x4, label_backward
0x114: blt  x5, x6, label_far
0x118: bge  x7, x8, label_forward
0x11c: lui  x9, 0xFFFFF
0x120: auipc x10, 0x7FFFF
0x124: jal  x1, label_forward
0x128: jalr x2, x3, -4
0x12c: add x0, x0, x0
0x130: add  x1, x2, x3
0x134: sub  x4, x5, x6
0x138: sll  x7, x8, x9
0x13c: slt  x10, x11, x12
0x140: sltu x13, x14, x15
0x144: xor  x16, x17, x18
0x148: srl  x19, x20, x21
0x14c: sra  x22, x23, x24
0x150: or   x25, x26, x27
0x154: and  x28, x29, x30
0x158: addi x1, x1, 0b11
0x15c: addi x2, x2, 2047
0x160: lw   x7, 1023(x8)
0x164: lb   x9, -128(x10)
0x168: sw   x1, -1024(x2)
0x16c: sb   x3, 127(x4)
0x170: beq  x1, x2, label_forward
0x174: bne  x3, x4, label_backward
0x178: blt  x5, x6, label_far
0x17c: bge  x7, x8, label_forward
0x180: lui  x9, 0xFFFFF
0x184: auipc x10, 0x7FFFF
0x188: jal  x1, label_forward
0x18c: jalr x2, x3, -4
0x190: add x0, x0, x0
0x194: add  x1, x2, x3
0x198: sub  x4, x5, x6
0x19c: sll  x7, x8, x9
0x1a0: slt  x10, x11, x12
0x1a4: sltu x13, x14, x15
0x1a8: xor  x16, x17, x18
0x1ac: srl  x19, x20, x21
0x1b0: sra  x22, x23, x24
0x1b4: or   x25, x26, x27
0x1b8: and  x28, x29, x30
0x1bc: addi x1, x1, 0b11
0x1c0: addi x2, x2, 2047
0x1c4: lw   x7, 1023(x8)
0x1c8: lb   x9, -128(x10)
0x1cc: sw   x1, -1024(x2)
0x1d0: sb   x3, 127(x4)
0x1d4: beq  x1, x2, label_forward
0x1d8: bne  x3, x4, label_backward
0x1dc: blt  x5, x6, label_far
0x1e0: bge  x7, x8, label_forward
0x1e4: lui  x9, 0xFFFFF
0x1e8: auipc x10, 0x7FFFF
0x1ec: jal  x1, label_forward
0x1f0: jalr x2, x3, -4
0x1f4: add x0, x0, x0
0x1f8: add  x1, x2, x3
0x1fc: sub  x4, x5, x6
0x200: sll  x7, x8, x9
0x204: slt  x10, x11, x12
0x208: sltu x13, x14, x15
0x20c: xor  x16, x17, x18
0x210: srl  x19, x20, x21
0x214: sra  x22, x23, x24
0x218: or   x25, x26, x27
0x21c: and  x28, x29, x30
0x220: addi x1, x1, 0b11
0x224: addi x2, x2, 2047
0x228: lw   x7, 1023(x8)
0x22c: lb   x9, -128(x10)
0x230: sw   x1, -1024(x2)
0x234: sb   x3, 127(x4)
0x238: beq  x1, x2, label_forward
0x23c: bne  x3, x4, label_backward
0x240: blt  x5, x6, label_far
0x244: bge  x7, x8, label_forward
0x248: lui  x9, 0xFFFFF
0x24c: auipc x10, 0x7FFFF
0x250: jal  x1, label_forward
0x254: jalr x2, x3, -4
0x258: add x0, x0, x0
0x25c: add  x1, x2, x3
0x260: sub  x4, x5, x6
0x264: sll  x7, x8, x9
0x268: slt  x10, x11, x12
0x26c: sltu x13, x14, x15
0x270: xor  x16, x17, x18
0x274: srl  x19, x20, x21
0x278: sra  x22, x23, x24
0x27c: or   x25, x26, x27
0x280: and  x28, x29, x30
0x284: addi x1, x1, 0b11
0x288: addi x2, x2, 2047
0x28c: lw   x7, 1023(x8)
0x290: lb   x9, -128(x10)
0x294: sw   x1, -1024(x2)
0x298: sb   x3, 127(x4)
0x29c: beq  x1, x2, label_forward
0x2a0: bne  x3, x4, label_backward
0x2a4: blt  x5, x6, label_far
0x2a8: bge  x7, x8, label_forward
0x2ac: lui  x9, 0xFFFFF
0x2b0: auipc x10, 0x7FFFF
0x2b4: jal  x1, label_forward
0x2b8: jalr x2, x3, -4
0x2bc: add x0, x0, x0
0x2c0: add  x1, x2, x3
0x2c4: sub  x4, x5, x6
0x2c8: sll  x7, x8, x9
0x2cc: slt  x10, x11, x12
0x2d0: sltu x13, x14, x15
0x2d4: xor  x16, x17, x18
0x2d8: srl  x19, x20, x21
0x2dc: sra  x22, x23, x24
0x2e0: or   x25, x26, x27
0x2e4: and  x28, x29, x30
0x2e8: addi x1, x1, 0b11
0x2ec: addi x2, x2, 2047
0x2f0: lw   x7, 1023(x8)
0x2f4: lb   x9, -128(x10)
0x2f8: sw   x1, -1024(x2)
0x2fc: sb   x3, 127(x4)
0x300: beq  x1, x2, label_forward
0x304: bne  x3, x4, label_backward
0x308: blt  x5, x6, label_far
0x30c: bge  x7, x8, label_forward
0x310: lui  x9, 0xFFFFF
0x314: auipc x10, 0x7FFFF
0x318: jal  x1, label_forward
0x31c: jalr x2, x3, -4
0x320: add x0, x0, x0
0x324: add  x1, x2, x3
0x328: sub  x4, x5, x6
0x32c: sll  x7, x8, x9
0x330: slt  x10, x11, x12
0x334: sltu x13, x14, x15
0x338: xor  x16, x17, x18
0x33c: srl  x19, x20, x21
0x340: sra  x22, x23, x24
0x344: or   x25, x26, x27
0x348: and  x28, x29, x30
0x34c: addi x1, x1, 0b11
0x350: addi x2, x2, 2047
0x354: lw   x7, 1023(x8)
0x358: lb   x9, -128(x10)
0x35c: sw   x1, -1024(x2)
0x360: sb   x3, 127(x4)
0x364: beq  x1, x2, label_forward
0x368: bne  x3, x4, label_backward
0x36c: blt  x5, x6, label_far
0x370: bge  x7, x8, label_forward
0x374: lui  x9, 0xFFFFF
0x378: auipc x10, 0x7FFFF
0x37c: jal  x1, label_forward
0x380: jalr x2, x3, -4

