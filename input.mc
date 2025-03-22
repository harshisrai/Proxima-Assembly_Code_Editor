Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x100002B7      lui x5 0                             # 0110111-NULL-NULL-00101-NULL-NULL-00000000000000000000
0x4:          0x10028293      addi x5 x5 0                         # 0010011-000-NULL-00101-00101-NULL-000000000000
0x8:          0xDEADC237      lui x4 0                             # 0110111-NULL-NULL-00100-NULL-NULL-00000000000000000000
0xc:          0xEEF20213      addi x4 x4 0                         # 0010011-000-NULL-00100-00100-NULL-000000000000
0x10:         0x0042A023      sw x4 0(x5)                          # 0100011-010-0000000-NULL-00101-00100-000000000000
0x8   #end of text segment

Memory Address   Value
0x10000000     0x00000001
0x10000004     0x00000002
0x10000008     0x00000003
0x1000000C     0x68
0x1000000D     0x65
0x1000000E     0x6C
0x1000000F     0x6C
0x10000010     0x6F


