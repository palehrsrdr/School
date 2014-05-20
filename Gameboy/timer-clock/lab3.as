@lab3 

.text
.align 2
.global main

@display
.set REG_DISPCNT, 0x04000000
.set REG_DISPSTAT, 0x04000004
.set REG_KEYINPUT, 0x04000130
.set VRAM, 0x06000000

.set BLACK, 0x0000
.set BLUE, 0x7C00
.set GREEN, 0x03E0
.set RED, 0x001F
.set WHITE, 0xFFFF

@buttons
.set BTN_A, 0x01
.set BTN_B, 0x02
.set BTN_RT, 0x10
.set BTN_LT, 0x20
.set BTN_UP, 0x40
.set BTN_DN, 0x80

@timers
.set T0, 0x4000100
.set T0D, 0x4000102
.set T1, 0x4000104
.set T1D, 0x4000106
.set T2, 0x4000108
.set T2D, 0x400010A
.set T3, 0x400010C
.set T3D, 0x400010E

.set TICKS, 256
.set MIN_CNT, 65476
.set SEC_CNT, 65280
 
main:
	ldr r0, =REG_DISPCNT
	ldr r1, =0x403
	strh r1, [r0]
	ldr r12, =VRAM

	ldr r2, =T3
	ldr r3, =MIN_CNT
	strh r3, [r2]

	ldr r2, =T3D
	ldr r3, =0x84
	strh r3, [r2]

	ldr r2, =T2
	ldr r3, =MIN_CNT
	str r3,[r2]

	ldr r2, =T2D
	ldr r3, =0x84
	strh r3, [r2]

	ldr r2, =T1
	ldr r3, =SEC_CNT
	strh r3, [r2]

	ldr r2, =T1D
	ldr r3, =0x84
	strh r3, [r2]

	ldr r2, =T0
	ldr r3, =0
	strh r3, [r2]

	ldr r2, =T0D
	ldr r3, =0x80
	strh r3, [r2]

	main_loop:

@@@@@@@@@@
@ BUTTON @
@@@@@@@@@@

		ldr r0, =REG_KEYINPUT
		ldrh r1, [r0]	
		mvn r0, #0

		@if B draw pixel at 40, 40
		mov r7, #65 @x
		mov r8, #50 @y
		ands r2, r1, #BTN_B
		ldreq r9, =WHITE
		ldrne r9, =RED
		bl draw_pixel

		@if b draw pixel at 60 ,40
		mov r7, #75 @x
		mov r8, #45  @y
		ands r2, r1, #BTN_A
		ldreq r9, =WHITE
		ldrne r9, =RED
		bl draw_pixel

		@ if up is pressed draw pixel at 119,50
		mov r7, #40 @x
		mov r8, #40 @y
		ands r2, r1, #BTN_UP
		ldreq r9, =WHITE
		ldrne r9, =RED
		bl draw_pixel

		@ if down draw pixel at 190, 100
		mov r7, #40 @x
		mov r8, #60 @y
		ands r2, r1, #BTN_DN
		ldreq r9, =WHITE
		ldrne r9, =RED
		bl draw_pixel

		@ if right draw pixel at 70,75
		mov r7, #50 @x
		mov r8, #50  @y
		ands r2, r1, #BTN_RT
		ldreq r9, =WHITE
		ldrne r9, =RED
		bl draw_pixel
	
		@ if left draw pixel at 30, 75
		mov r7, #30 @x
		mov r8, #50  @y
		ands r2, r1, #BTN_LT
		ldreq r9, =WHITE
		ldrne r9, =RED
		bl draw_pixel

		



@@@@@@@@@
@ CLOCK @
@@@@@@@@@

		bl d_seconds
		bl d_minutes

		@fetch elapsed minutes
		ldr r0, =T3
		ldr r0, [r0]

		@compare min values
		cmp r0, r11
		movne r11, r0
		blne clear_seconds
	b main_loop

clear_seconds:
	stmfd r13!, {r0-r11, r14}
	adrl r0, seconds
	mov r1, #60

	loop:
		ldrh r2, [r0]
		and r7, r2, #0xff
		and r8, r2, #0xff00
		mov r8, r8, lsr #8
		eor r9,r9

		bl draw_pixel
		add r0, r0, #2
		subs r1, r1, #1
		bne loop
	ldmfd r13!, {r0-r11,r14}
	mov r15, r14

draw_pixel:
	@ r0 - pixel value
	stmfd r13!, {r1-r11, r14}
	
	@load the y value
	@ldr r4, r8
	
	@width
	mov r10, #240
	@will use later...
	mov r11, #2
	
	@multiply the Y position by the width and store it in r2
	mul r2, r8, r10
	
	@Adding the x value to it.
	add r2, r2, r7
	
	@multiplying by 2 (store in r11)
	mul r3, r2, r11
	
	@putting the color into the VRAM plus the offset for the pixel position
	strh r9, [r12, r3]
	
	ldmfd r13!, {r1-r11, r14}
	mov r15, r14	

fill_screen:

	stmfd r13!, {r1-r11, r14}
	
	
	ldr r7, =0x06013FFF
	ldr r1, =VRAM
	
	ldr r9, =BLUE
	
	fill_screen_loop:
		strh r9, [r1]
		add r1, #2
		cmp r1, r7
		blt fill_screen_loop	
	
	ldmfd r13!, {r1-r11, r14}
	mov r15, r14	

d_seconds:
	stmfd r13!, {r1-r11, r14}
	adrl r0, seconds

	ldr r1, =T2
	ldrh r2, [r1]
	ldr r3, =MIN_CNT

	sub r4, r2, r3
	mov r4, r4, lsl #1

	draws:
		ldrh r5, [r0,r4]
		and r7, r5, #0xff
		mov r5, r5, lsr #8
		and r8, r5, #0xff

		ldr r9, =BLUE
		bl draw_pixel

		sub r4, r4, #16
		cmp r4, #0
		blgt draws

	ldmfd r13!, {r1-r11, r14}
	mov r15, r14

d_minutes:
	stmfd r13!, {r1-r11, r14}
	adrl r0, minutes
	
	ldr r1, =T3
	ldrh r2, [r1]
	ldr r3, =MIN_CNT

	sub r4, r2, r3
	mov r4, r4, lsl #1
	drawm:
		ldrh r5, [r0, r4]
		and r7, r5, #0xff
		mov r5, r5, lsr #8
		and r8, r5, #0xff

		ldr r9, =WHITE
		bl draw_pixel

		sub r4, r4, #16
		cmp r4, #0
		blgt drawm
	ldmfd r13!, {r1-r11, r14}
	mov r15, r14

.end