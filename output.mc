Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x00100093      addi x1 x0 1                         # 0010011-000-NULL-00001-00000-NULL-000000000001
0x4:          0x0140006F      jal x0  t4                           # 1101111-NULL-NULL-00000-NULL-NULL-00000000000000010100
0x8:          0x00100113      addi x2 x0 1                         # 0010011-000-NULL-00010-00000-NULL-000000000001
0xc:          0x0140006F      jal x0  t5                           # 1101111-NULL-NULL-00000-NULL-NULL-00000000000000010100
0x10:         0x00100193      addi x3 x0 1                         # 0010011-000-NULL-00011-00000-NULL-000000000001
0x14:         0x0140006F      jal x0  exit                         # 1101111-NULL-NULL-00000-NULL-NULL-00000000000000010100
0x18:         0x00100213      addi x4 x0 1                         # 0010011-000-NULL-00100-00000-NULL-000000000001
0x1c:         0xFEDFF06F      jal x0  t2                           # 1101111-NULL-NULL-00000-NULL-NULL-11111111111111101100
0x20:         0x00100293      addi x5 x0 1                         # 0010011-000-NULL-00101-00000-NULL-000000000001
0x24:         0xFEDFF06F      jal x0  t3                           # 1101111-NULL-NULL-00000-NULL-NULL-11111111111111101100
0x8   #end of text segment

Memory Address   Value
