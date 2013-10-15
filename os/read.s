.rep 64
	nop
.endr

read_sec:
	pusha

	//step 1
	mov $0x1f7
	notready:
		in %dx, %al
		test $0x80
		jnz notready

	// step 2
	mov $0x36f, %dx
	mov $2, %al
	out %al, %dx

	// step 3
	mov $0x1f2, %dx
	mov $1, %al
	out %al, %dx

	// step 4

	// step 5

	// step 6

	// step 7
	bit7:
    	in %dx, %al
	    test $0x80, %al
	    jnz bit7

	// step 8
		mov $256, %ecx
		mov $0x1f0, %dx
		readloop:
			in %dx, %ax
			mov %ax, (%ebx)
			add $2, %ebx
			sub $1, %ecx
			jnz readloop


mov $0x7e00, %eax  //p
mov $0xb8000, %ebx //s
mov $0, %ecx

ffl:
	cmp $5, %ecx
	je effl
	mov $0, edi
	sffl:
		cmp $11, %edi
		je esfl
		mov (%eax), %dl
		mov %dl, (%ebx)
		inc %edx
		movb %0x1f, (%ebx)
		inc %ebx
		inc %eax
		inc %edi
		jmp sfl
	esfl:
		add $0xa3, %eax
		add $0x8a, %ebx
		inc %ecx
		jmp ffl
effl:
	popa