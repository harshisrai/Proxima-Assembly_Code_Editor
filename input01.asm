
addi t3, x0, 0x100
slli t3, t3, 20

keep_looping: addi t0, x0, 0
addi t1, x0, 1
sw t0, 0(a2)
addi a1, a1, -1
addi a2, a2, 4
sw t1, 0(a2)
addi a1, a1, -1
add t2, t1, t0
addi a2, a2, 4
sw t2, 0(a2)
addi a1, a1, -1
jal x0 keep_looping
