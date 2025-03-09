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

