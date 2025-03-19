Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x00500293      addi x5 x0 5                         # 0010011-000-NULL-00101-00000-NULL-000000000101
0x4:          0x00A00313      addi x6 x0 10                        # 0010011-000-NULL-00110-00000-NULL-000000001010
0x8:          0x02536FB3      rem x31 x6 x5                        # 0110011-110-0000001-11111-00110-00101-NULL
0xc:          0x000F8463      beq x31 x0 branch1                   # 1100011-000-0000000-NULL-11111-00000-000000001000
0x10:         0x01F2AEB3      slt x29 x5 x31                       # 0110011-010-0000000-11101-00101-11111-NULL
0x14:         0x00531F33      sll x30 x6 x5                        # 0110011-001-0000000-11110-00110-00101-NULL
0x8   #end of text segment

Memory Address   Value
