<<<<<<< HEAD
addi x3 x0 5
addi x4 x0 10
add  x5 x3 x4
addi x6 x3 10
beq  x5 x6 target
addi x7 x0 30
beq  x0 x0 exit
addi x3 x0 5
beq x8 x1 target
beq x9 x2 target
addi x3 x0 5
target: addi x7 x0 20
=======
addi x4 x0 5
add x5 x0 x0 
label:
addi x5 x5 1
bne x4 x5 label
add x7 x5 x0
>>>>>>> 13b9e2a56013f695cdb6ed29c2ceb498762854dd

