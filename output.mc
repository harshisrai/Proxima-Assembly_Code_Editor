Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x00500213      addi x4 x0 5                         # 0010011-000-NULL-00100-00000-NULL-000000000101
0x4:          0x000002B3      add x5 x0 x0                         # 0110011-000-0000000-00101-00000-00000-NULL
0x8:          0x00128293      addi x5 x5 1                         # 0010011-000-NULL-00101-00101-NULL-000000000001
0xc:          0xFE521EE3      bne x4 x5 label                      # 1100011-001-0000000-NULL-00100-00101-111111111100
0x10:         0x000283B3      add x7 x5 x0                         # 0110011-000-0000000-00111-00101-00000-NULL
0x8   #end of text segment

Memory Address   Value
