# Venus RISC-V Assembly - Edge Cases and Syntax Exploration

.data  # Data Section

.text  # Code Section

.globl _start
_start: 

    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   
    # R-type instructions
    label2:add x0, x0, x0
    add  x1, x2, x3   # Addition
    sub  x4, x5, x6   # Subtraction
    sll  x7, x8, x9   # Shift left
    slt  x10, x11, x12  # Set if less than
    sltu x13, x14, x15  # Unsigned set if less than
    xor  x16, x17, x18  # XOR
    srl  x19, x20, x21  # Shift right
    sra  x22, x23, x24  # Shift right arithmetic
    or   x25, x26, x27  # OR
    and  x28, x29, x30  # AND

    # I-type instructions
    addi x1, x1, 0b11  # Edge case: min imm
    addi x2, x2, 2047   # Edge case: max imm
   
    
  
    lw   x7, 1023(x8)   # Load edge offset
    lb   x9, -128(x10)  # Load negative offset
   

    # S-type instructions (store)
    sw   x1, -1024(x2)  # Store edge case
    sb   x3, 127(x4)    # Store byte max offset

    # B-type instructions (branches)
    beq  x1, x2, label_forward   # Branch forward
    bne  x3, x4, label_backward  # Branch backward
    blt  x5, x6, label_far       # Branch to far location
    bge  x7, x8, label_forward

    # U-type instructions (lui, auipc)
    lui  x9, 0xFFFFF    # Load max upper immediate
    auipc x10, 0x7FFFF  # Upper immediate edge case

    # J-type instructions (jump)
    jal  x1, label_forward  # Jump forward
    jalr x2, x3, -4         # Indirect jump


   