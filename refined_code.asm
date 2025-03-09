<<<<<<< HEAD
0x0: addi t3  x0  0x100
0x4: slli t3  t3  20
0x8: addi t0  x0  0
0xc: addi t1  x0  1
0x10: sw t0  0(a2)
0x14: addi a1  a1  -1
0x18: addi a2  a2  4
0x1c: sw t1  0(a2)
0x20: addi a1  a1  -1
0x24: add t2  t1  t0
0x28: addi a2  a2  4
0x2c: sw t2  0(a2)
0x30: addi a1  a1  -1
0x34: jal x0 keep_looping
=======
0x0: slli t3  t3  20
0x4:  addi t0  x0  0
0x8: addi t1  x0  1
0xc: sw t0  0(a2)
0x10: addi a1  a1  -1
0x14: addi a2  a2  4
0x18: sw t1  0(a2)
0x1c: addi a1  a1  -1
0x20: add t2  t1  t0
0x24: addi a2  a2  4
0x28: sw t2  0(a2)
0x2c: addi a1  a1  -1
0x30: jal x0 keep_looping
>>>>>>> 3869c3a6edf6080d71d353ad609c380cb2aab823

