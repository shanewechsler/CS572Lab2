
	.text

li $2 32
li $1, 0
nop

subi $2, $2, 1
nop
nop
nop
nop
nop
bge  $2, $1, -36
nop
li $20, 10
syscall