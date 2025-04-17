Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x00500193      addi x3 x0 5                         # 0010011-000-NULL-00011-00000-NULL-000000000101
0x4:          0x00A00213      addi x4 x0 10                        # 0010011-000-NULL-00100-00000-NULL-000000001010
0x8:          0x004182B3      add x5 x3 x4                         # 0110011-000-0000000-00101-00011-00100-NULL
0xc:          0x00A18313      addi x6 x3 10                        # 0010011-000-NULL-00110-00011-NULL-000000001010
0x10:         0x00628663      beq x5 x6 target                     # 1100011-000-0000000-NULL-00101-00110-000000001100
0x14:         0x01E00393      addi x7 x0 30                        # 0010011-000-NULL-00111-00000-NULL-000000011110
0x18:         0x00000463      beq x0 x0 exit                       # 1100011-000-0000000-NULL-00000-00000-000000001000
0x1c:         0x01400393      addi x7 x0 20                        # 0010011-000-NULL-00111-00000-NULL-000000010100
0x8   #end of text segment

Memory Address   Value
