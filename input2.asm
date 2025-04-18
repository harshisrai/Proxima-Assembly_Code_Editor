addi x3 x0 5
addi x4 x0 10
add  x5 x3 x4
addi x6 x3 10
bne  x5 x6 target
addi x7 x0 30
jal x0 exit
addi x3 x0 5
addi x3 x0 5
target: addi x7 x0 20

exit: 
