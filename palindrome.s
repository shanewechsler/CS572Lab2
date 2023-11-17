    .data
    .space 1024
The string is a palindrome.
The string is not a palindrome.
    
    .text

la $10 786424
li $11 1024
li $20 8
syscall

la $1 786408
la $2 786404

lb $3 0($2)
nop
beqz $3 12
nop
addi $2 $2 1
b -28
nop

subi $2 $2 1

bge $1 $2 40
nop

lb $3 0($1)
nop
lb $4 0($2)
nop
bne $3 $4 32
nop

addi $1 $1 1
subi $2 $2 1
b -48
nop

la $10 787344
li $20 4
syscall
b 16
nop

la $10 787356
li $20 4
syscall
b 0
nop

li $20 10
syscall
