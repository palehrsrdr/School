mov $_sbss, %ebx
mov $_ebss, %ecx

wipebss:
	cmp %ebx, %ecx
	je donewiping
	movb $0, (%ebx)
	inc %ebx
	jmp wipebss

donewiping:
	mov $0x90000, %esp
	#ifdef WIN_32
		call _main
	#else
		call main
	#endif
	hlt
