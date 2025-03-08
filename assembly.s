.data
labelword:.word 5 6 7
labelhalf:.half 6 7 8
labelbyte:.byte 0 1 2

.text
add x1 x2 x3
addi x2 x0 1
add x3 x2 x2
or x4 x2 x3
#harsh 


sll x5 x3 x2 
slt x5 x3 x6
sra x2 x3 x4
srl x2 x3 x4
sub x4 x5 x6
xor x31 x6 x1
mul x8 x31 x4
div x30 x8 x7
rem x29 x30 x31
andi x28 x29 -100
ori x27 x5 69
jal x15 branch1
addi x16 x0 1
branch1:auipc x16 65536
sb x8 0 x16
sw x5 6 x16
sh x30 -40 x16
beq x5 x6 branch1
bne x5 x6 branch2
branch2:bge x8 x9 branch3
branch3:blt x13 x14 branch4
branch4:
lui x6  10000
jalr x1 x15 0