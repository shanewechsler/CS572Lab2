    .data
    ahahahahahahaha\n
    The string is a palindrome.\n
    The string is not a palindrome.\n
    
    .text

    la $10 786482
    li $11 1024
    li $20 8
    syscall

    la $1 786482
    la $2 786482

    lb $3 0($2)
    beqz $3 8
    addi $2 $2 1
    b -16

    subi $2 $2 2

    bge $1 $2 18
    lb $3 0($1)
    lb $4 0($2)
    bne $3 $4 28
    addi $1 $1 1
    subi $2 $2 1
    b -28

    la $10 787380
    li $20 4
    syscall
    b 10

    la $10 788404
    li $20 4
    syscall
    b 0

    li $20 10
    syscall
