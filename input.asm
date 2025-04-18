.data
.asciz "HaahdhaH"

.text
addi x31 x0 8 # size of string
addi x10 x0 0 # initial left pointer
lui x30 0x10000
jal  x1 palindrome
addi x17 x16 5
beq x0 x0 exit

palindrome:
addi x2 x2 -4
sw x1 0 x2
sub x12 x31 x10
addi x12 x12 -1 # current right pointer
bge x10 x12 true
add x13 x30 x10
lb x14 0 x13
add x15 x30 x12
lb x16 0 x15
bne x16 x14 false
addi x10 x10 1
jal x1 palindrome
lw x1 0 x2
addi x2 x2 4
jalr x0 x1 0


true:
addi x20 x0 1
lw x1 0 x2
addi x2 x2 4
jalr x0 x1 0

false:
addi x20 x20 0
lw x1 0 x2
addi x2 x2 4
jalr x0 x1 0

exit: