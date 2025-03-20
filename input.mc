Address       Machine Code    Assembly Code 					   Opcode-Func3-Func7-rd-rs1-imm
0x0:          0x007302B3      add t0  t1  t2                       # 0110011-000-0000000-00101-00110-00111-NULL
0x4:          0x41EE8E33      sub t3  t4  t5                       # 0110011-000-0100000-11100-11101-11110-NULL
0x8:          0x00528F93      addi t6  t0  5                       # 0010011-000-NULL-11111-00101-NULL-000000000101
0xc:          0x000EAF03      lw t5  0(t4)                         # 0000011-010-NULL-11110-11101-NULL-000000000000
0x10:         0x12345537      lui a0  0x12345                      # 0110111-NULL-NULL-01010-NULL-NULL-00010010001101000101
0x14:         0x00010597      auipc a1  0x10                       # 0010111-NULL-NULL-01011-NULL-NULL-00000000000000010000
0x18:         0x008000EF      jal x1  func                         # 1101111-NULL-NULL-00001-NULL-NULL-00000000000000001000
0x1c:         0x0040006F      jal x0  func                         # 1101111-NULL-NULL-00000-NULL-NULL-00000000000000000100
0x20:         0x01C3A023      sw t3  0(t2)                         # 0100011-010-0000000-NULL-00111-11100-000000000000
0x24:         0x006280A3      sb t1  1(t0)                         # 0100011-000-0000000-NULL-00101-00110-000000000001
0x28:         0x01EE8463      beq t4  t5  label                    # 1100011-000-0000000-NULL-11101-11110-000000001000
0x2c:         0xFFCF9AE3      bne t6  t3  func                     # 1100011-001-0000000-NULL-11111-11100-111111110100
0x8   #end of text segment

Memory Address   Value
0x10000000     0x0000000A
0x10000004     0x00000014