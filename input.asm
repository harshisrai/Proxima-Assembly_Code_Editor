.data
input: .word 0
len: .word 3
arr: .word 30 50 20

.text
auipc x1 65536
addi x1 x1 0
lw x2 0 x1
auipc x10 65536
lw x10 -8 x10
auipc x11 65536
addi x11 x11 -12
addi x13 x13 4
mul x12 x10 x13
sub x12 x12 x13
add x12 x11 x12 #upper limit for loop
lui x3 65536 #addr to store sorted arr
addi x3 x3 1280
bne x2 x0 optimized
add x9 x0 x0 #flag for optimized

unoptimized:
    add x4 x0 x0 #counter
    add x5 x0 x11 #start
    loop1:
        beq x4 x10 end
        loop2:
            beq x5 x12 end1
            lw x6 0 x5
            lw x7 4 x5
            bge x7 x6 end2
            sw x7 0 x5
            sw x6 4 x5
            end2:
            addi x5 x5 4
            beq x0 x0 loop2
            end1:
            add x5 x0 x11
            addi x4 x4 1
            beq x0 x0 loop1

optimized:
    add x4 x0 x0 #counter
    add x5 x0 x11 #start
    loop3:
        beq x4 x10 end
        sub x14 x10 x4
        addi x14 x14 -1
        loop4:
            beq x8 x14 end3
            lw x6 0 x5
            lw x7 4 x5
            bge x7 x6 end4
            sw x7 0 x5
            sw x6 4 x5
            addi x9 x9 1
            end4:
            addi x5 x5 4
            addi x8 x8 1
            beq x0 x0 loop4
            end3:
            beq x9 x0 end #break loop if flag is set to 0
            add x5 x0 x11
            add x8 x0 x0
            addi x4 x4 1
            beq x0 x0 loop3
    
    beq x0 x0 end
end:.