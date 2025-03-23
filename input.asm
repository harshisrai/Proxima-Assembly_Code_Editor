.data
labelword:.word -279311378 6 775
labelhalf:.half 6 7 8
labelbyte:.byte 0 1 2
.asciz "InShallah !"

.text
addi x5 x0 5
addi x6 x0 6
lui x7 0x10000

addi x7 x7 4
sll x8 x5 x6
sw x8 0 x7
lw x9 1 x7
auipc x10 0x10000
addi x10 x10 -28
lh x15 20 x10
lb x11 0 x10
lb x12 1 x10
sb x12 5 x7

jal x1 label1
sh x31 10  x7
bne x5 x6 end

label1:
beq x5 x6 label2
and x31 x5 x6
jalr x0 x1 0

label2:
slli x31 x5 5

end: or x20 x5 x6