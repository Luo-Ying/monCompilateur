			# This code was produced by the CERI Compiler
.data
FormatString1:	.string "%llu"	# used by printf to display 64-bit unsigned integers
FormatString2:	.string "%lf"	# used by printf to display 64-bit floating point numbers
FormatString3:	.string "%c"	# used by printf to display a 8-bit single character
TrueString:	.string "TRUE"	# used by printf to display the boolean value TRUE
FalseString:	.string "FALSE"	# used by printf to display the boolean value FALSE
a:	.quad 0
i:	.quad 0
b:	.double 0.0
s:	.double 0.0
c:	.quad 0
o:	.byte 0
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $0
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
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $0, (%rsp)	# Conversion of 2.5 (32 bit high part)
	movl $1074003968, 4(%rsp)	# Conversion of 2.5 (32 bit low part)
	pop s
	movq $0,%rax
	movb $'c',%al
	push %rax	# push a 64-bit version of 'c'
	pop %rax
	movb %al,o
	pop o
While2:
	push a
	push $3
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	jb Vrai5	# If below
	push $0		# False
	jmp Suite5
Vrai5:	push $0xFFFFFFFFFFFFFFFF		# True
Suite5:
	pop %rax	# Get the result of expression
	cmpq $0,%rax
	je suite2	# if FALSE, jump out of the loop2
	push a
	push $5
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	je Vrai8	# If equal
	push $0		# False
	jmp Suite8
Vrai8:	push $0xFFFFFFFFFFFFFFFF		# True
Suite8:
	pop %rax	# Get the result of expression
	cmpq $0,%rax
	je Else5	# if FALSE ,jump to Else5
	push a
	push $2
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop a
	jmp Next5	#Do not execute the else statement
Else5:
	push a
	push $1
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop a
Next5:
	push a
	push $1
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop a
	jmp While2
suite2:
	push a
	push $10
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	je Vrai11	# If equal
	push $0		# False
	jmp Suite11
Vrai11:	push $0xFFFFFFFFFFFFFFFF		# True
Suite11:
	pop %rax	# Get the result of expression
	cmpq $0,%rax
	je Else8	# if FALSE ,jump to Else8
	push a
	push $2
	pop %rbx
	pop %rax
	mulq	%rbx
	push %rax	# MUL
	pop a
	jmp Next8	#Do not execute the else statement
Else8:
	push a
	push $1
	pop %rbx
	pop %rax
	addq	%rbx, %rax	# ADD
	push %rax
	pop a
Next8:
	push $1
	pop i
for11:
	push $2
	pop %rax
	cmpq %rax,i
	je SuiteFor11
	push s
	subq $8,%rsp			# allocate 8 bytes on stack's top
	movl $2061584302, (%rsp)	# Conversion of 0.99 (32 bit high part)
	movl $1072672276, 4(%rsp)	# Conversion of 0.99 (32 bit low part)
	fldl 8(%rsp)	
	fldl (%rsp)	# first oprand -> %st(0) ; second operand -> %st(1)
	fmulp %st(0),%st(1)	# %st(0) <- op1 + op2 ; %st(1)=null
	fstpl 8(%rsp)
	addq $8,%rsp	# result on stack's top
	pop s
	addq $1,i
	push %rax
	pop i
	jmp for11
SuiteFor11:
	push s
	movsd (%rsp),%xmm0		# &stack top -> %xmm0
	subq $16,%rsp		# allocation for 3 additional doubles
	movsd %xmm0,8(%rsp)
	movq $FormatString2, %rdi	# "%lf\n"
	movq $1,%rax
	call printf@PLT
nop
	addq $24,%rsp			# pop nothing
	push c
	pop %rdx	# Zero : False, non-zero : true
	cmpq $0,%rdx
	je False14
	movq $TrueString,%rdi	# "TRUE\n"
	jmp Next14
False14:
	movq $FalseString,%rdi	# "FALSE\n"
Next14:
	call puts
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
