<<<<<<< HEAD
=======
#/* 
#Fibonacci sequence generator
#Results are stored from 0x80001000 as series of 32bits word each.
#fn = f(n-1) + f(n-2);
#t2 = t1 + t0 -> Registers 
#*/
.data
Len: .word 5
DEST_MEM_ADDR: .word 0x10000100

.text
main: 
>>>>>>> 53d2ded58d1b4380fb0315c83866459bb7af998c
addi t3, x0, 0x100
slli t3, t3, 20
lw a1, 0, t3
lw a2, 4, t3

<<<<<<< HEAD
keep_looping:addi t0, x0, 0
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
=======
#li a1, SEQ_LEN
#li a2, DEST_MEM_ADDR

addi t0, x0, 0
addi t1, x0, 1

#blez a1, keep_looping 
#    /* Fibonacci seq len is >= 1; Write the first number 0 */
    sw t0, 0(a2)
    addi a1, a1, -1

#blez a1, keep_looping
#    /* Fibonacci seq len is >= 2; Write the second number 1 */
    addi a2, a2, 4
    sw t1, 0(a2)
    addi a1, a1, -1

##blez a1, keep_looping
# /* Fibonacci seq len is >= 3; Write the other numbers that follow the sequence */
Fibonacci:

    add t2, t1, t0
    addi a2, a2, 4
    sw t2, 0(a2)

    #mv t0, t1
    #mv t1, t2
    addi a1, a1, -1

#bnez a1, Fibonacci

keep_looping: 
#nop
j keep_looping
>>>>>>> 53d2ded58d1b4380fb0315c83866459bb7af998c
