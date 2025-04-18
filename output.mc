Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x00500193      addi x3 x0 5                         # 0010011-000-NULL-00011-00000-NULL-000000000101
0x4:          0x000002B3      add x5 x0 x0                         # 0110011-000-0000000-00101-00000-00000-NULL
0x8:          0x00A00213      addi x4 x0 10                        # 0010011-000-NULL-00100-00000-NULL-000000001010
0xc:          0x00418E33      add  x28 x3 x4                       # 0110011-000-0000000-11100-00011-00100-NULL
0x10:         0x01C282B3      add x5 x5 x28                        # 0110011-000-0000000-00101-00101-11100-NULL
0x14:         0x00A18313      addi x6 x3 10                        # 0010011-000-NULL-00110-00011-NULL-000000001010
0x18:         0xFE6288E3      beq  x5 x6 target                    # 1100011-000-0000000-NULL-00101-00110-111111110000
0x1c:         0x01E00393      addi x7 x0 30                        # 0010011-000-NULL-00111-00000-NULL-000000011110
0x20:         0x0100006F      jal x0 exit                          # 1101111-NULL-NULL-00000-NULL-NULL-00000000000000010000
0x24:         0x00500193      addi x3 x0 5                         # 0010011-000-NULL-00011-00000-NULL-000000000101
0x28:         0x00500193      addi x3 x0 5                         # 0010011-000-NULL-00011-00000-NULL-000000000101
0x2c:         0x01400393      addi x7 x0 20                        # 0010011-000-NULL-00111-00000-NULL-000000010100
0x8   #end of text segment

Memory Address   Value
