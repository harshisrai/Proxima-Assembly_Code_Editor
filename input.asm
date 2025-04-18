t1:
addi x1 x0 1
jal x0  t4
t2:
addi x2 x0 1
jal x0  t5
t3:
addi x3 x0 1
jal x0  exit
t4:
addi x4 x0 1
jal x0  t2
t5:
addi x5 x0 1
jal x0  t3
exit: