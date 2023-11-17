
	.text

li $2 32
li $1, 0
subi $2, $2, 1
bge  $2, $1, -12
nop
li $20, 10
syscall