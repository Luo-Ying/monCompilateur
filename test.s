			# This code was produced by the CERI Compiler
a:	.quad 0
b:	.double 0.0
c:	.quad 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $5
	pop a
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $858993459, (%rsp)	# Conversion of 1.2 (32 bit high part)
	movl $1072902963, 4(%rsp)	# Conversion of 1.2 (32 bit low part)
	pop b
	push $3
	push $4
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	ja Vrai2	# If above
	push $0		# False
	jmp Suite2
Vrai2:	push $0xFFFFFFFFFFFFFFFF		# True
Suite2:
	pop c
	push a
	push $5
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop a
	push b
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $2576980378, (%rsp)	# Conversion of 1.1 (32 bit high part)
	movl $1072798105, 4(%rsp)	# Conversion of 1.1 (32 bit low part)
	fldl 8(%rsp)	
	fldl (%rsp)	# first oprand -> %st(0) ; second operand -> %st(1)
	fmulp %st(0),%st(1)	# %st(0) <- op1 + op2 ; %st(1)=null
	fstpl 8(%rsp)
	addq $8,%rsp	# result on stack's top
	pop b
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
