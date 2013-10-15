jmp init

.rep 64
	nop
.endr

init:
	mov $0, %edx

cls:
	movb $32, 0xb8000(%edx)
	movb $0x0, 0xb8001(%edx)
	add $2, %edx
	cmp $0xb8f9e, %edx
	jl cls

title:
    movb $74, (0xb8546)         
    movb $0x0f, (0xb8547)
    movb $73, (0xb8548)         
    movb $0x0f, (0xb8549)
    movb $77, (0xb854a)         
    movb $0x0f, (0xb854b)
    movb $32, (0xb854c)
    movb $0x0f,(0xb854d)
    movb $79, (0xb854e)
    movb $0x0f,(0xb854f)
    movb $83, (0xb8550)
    movb $0x0f, (0xb8551)






mov (0x7c00 + 28), %ecx
mov $0, %edi
mov (0x7c00 + 22), %di
inc %ecx
add %edi, %ecx
add %edi, %ecx
mov $0x7e00, %ebx

call read_sector

mov (0x7e00 + 28), %esi

shr $9, %esi
inc %esi
mov $0, %eax
mov (0x7c00 + 17), %ax
and $0xffff, %eax
shl $5, %eax
shr $9, %eax
add %eax, %ecx

mov $0, %eax
ploop:

    call read_sector
    inc %ecx
    add $512, %ebx
    inc %eax
    cmp %eax, %esi
    je end
    jmp ploop

end:

    mov $0, %eax
    mov $0x7e00, %eax
    
    jmp *%eax

read_sector:
    pusha
    # step 1
    mov $0x1f7,%dx
    notready:
        in %dx,%al         
        test $0x80,%al
        jnz notready
    # step 2
    mov $0x3f6,%dx
    mov $2,%al 
    out %al,%dx

    # step 3
    mov $0x1f2, %dx
    mov $1,%al              
    out %al,%dx

    #step 4
    mov $0x1f3, %dx
    mov %ecx, %eax
    out %al, %dx
    shr $8, %eax
    mov $0x1f4, %dx
    out %al, %dx
    shr $8, %eax
    mov $0x1f5, %dx
    out %al, %dx
    mov $0x1f6, %dx
    shr $8, %eax
    or $0xe0, %al
    out %al, %dx


    # step 5
    mov $0x20, %al
    mov $0x1f7, %dx
    out %al, %dx

    # step 6
   
    bit7:
        in %dx,%al         
        test $0x80,%al
        jnz bit7
        mov $0x1f7,%dx
    # step 7
    bit3:
        in %dx,%al
        test $0x8, %al
        jz bit3


    # step 8
    mov $0, %ecx
    mov $256, %ecx
    mov $0x1f0, %dx
    

    readloop:
        in %dx,%ax
        mov %ax,(%ebx)              
        add $2,%ebx
        sub $1,%ecx
        cmp $0,%ecx    
        jnz readloop
    popa
ret