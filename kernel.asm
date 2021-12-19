	ads r1a r1e 0
	ads r18 r1a STACK
	ads r19 r18 0
	ads r1b r1e 2
	jnc r1f r1a MAIN
	xor r1f r1f r1f

Mbox
	0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

exception_handler
	ads r18 r18 2
	str r1b r18 0
	str r1d r18 -1
	ldr r1d r18 -4
	ads r1d r1d 30
	ads r1b r1e 2
	jnc r1f r1a uart_send
	ldr r1d r18 -1
	ldr r1b r18 0
	ads r18 r18 -2
	exc -1

mbox_call
	ads r18 r18 3
	str r19 r18 -2
	ads r19 r18 -2
	str r1 r19 1
	str r2 r19 2

	ads r1 r1a Mbox
	shf r1 r1 -3
	xor r2 r2 r2
	ads r2 r2 F
	nor r2 r2 r2
	and r1 r1 r2
	and r2 r1d F
	orr r1 r1 r2

	xor r2 r2 r2
	ads r2 r2 7e01710

	ldr r1d r2 3
	and r1d r1d 80000000
	jnc r1d r1e -2

	ldr r1d r2 4
	shf r1d r1d 20
	shf r1d r1d -20
	orr r1d r1 r1d
	str r1d r2 4

	ldr r1d r2 3
	and r1d r1d 40000000
	jnc r1d r1e -2

	ldr r1d r2 0
	and r1d r1d ffffffff
	xor r1d r1 r1d
	jnc r1d r1e -6

	ldr r1d r1a Mbox
	shf r1d r1d 20
	xor r1d r1d 80000000
	jnc r1d r1e 3
	ads r1d r1d 1
	jnc r1f r1e 2
	xor r1d r1d r1d

	ldr r2 r19 2
	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 3
	jnc r1F r1b 0


fb_init
	ads r18 r18 4
	str r19 r18 -3
	ads r19 r18 -3
	str r1 r19 1
	str r2 r19 2
	str r3 r19 3

	xor r3 r3 r3
	ads r1 r1a Mbox
	ads r2 r3 8c
	str r2 r1 0
	ads r2 r3 8
	shf r2 r2 -20
	ads r2 r2 48003
	str r2 r1 1
	ldr r2 r1d 0
	shf r2 r2 -20
	ads r2 r2 8
	str r2 r1 2
	ads r2 r3 48004
	shf r2 r2 -20
	ldr r3 r1d 1
	orr r2 r2 r3
	str r2 r1 3
	xor r3 r3 r3
	ads r2 r3 8
	shf r2 r2 -20
	ads r2 r2 8
	str r2 r1 4
	ldr r2 r1d 0
	ldr r3 r1d 1
	shf r3 r3 -20
	orr r2 r2 r3
	str r2 r1 5
	xor r3 r3 r3
	ads r2 r3 8
	shf r2 r2 -20
	ads r2 r2 48009
	str r2 r1 6
	ads r2 r3 8
	str r2 r1 7
	ads r2 r3 48005
	shf r2 r2 -20
	str r2 r1 8
	ads r2 r3 4
	shf r2 r2 -20
	ads r2 r2 4
	str r2 r1 9
	ads r2 r3 48006
	shf r2 r2 -20
	ads r2 r2 20
	str r2 r1 a
	ads r2 r3 4
	shf r2 r2 -20
	ads r2 r2 4
	str r2 r1 b
	ads r2 r3 40001
	shf r2 r2 -20
	ads r2 r2 1
	str r2 r1 c
	ads r2 r3 8
	shf r2 r2 -20
	ads r2 r2 8
	str r2 r1 d
	ads r2 r3 1000
	str r2 r1 e
	ads r2 r3 4
	shf r2 r2 -20
	ads r2 r2 40008
	str r2 r1 f
	ads r2 r3 4
	str r2 r1 10
	str r3 r1 11

	ads r2 r1d 0
	xor r1d r1d r1d
	ads r1d r1d 8
	ads r18 r18 1
	str r1b r18 0
	ads r1b r1e 2
	jnc r1f r1a mbox_call
	ldr r1b r18 0
	ads r18 r18 -1
	jnc r1d r1e 2
	jnc r1f r1a fb_init_cleanup

	ldr r3 r1 a
	shf r3 r3 -20
	shf r3 r3 20
	xor r3 r3 20
	jnc r3 r1a fb_init_cleanup

	ldr r3 r1 e
	shf r3 r3 -20
	shf r3 r3 20
	jnc r3 r1e 2
	jnc r1f r1a fb_init_cleanup

	and r3 r3 3fffffff
	ads r1d r2 0
	ldr r2 r1d 6
	str r3 r2 0
	ldr r2 r1d 2
	ldr r3 r1 2
	shf r3 r3 20
	str r3 r2 0
	ldr r2 r1d 3
	ldr r3 r1 3
	shf r3 r3 20
	shf r3 r3 -20
	str r3 r2 0
	ldr r2 r1d 4
	ldr r3 r1 10
	shf r3 r3 20
	str r3 r2 0
	ldr r2 r1d 5
	ldr r3 r1 c
	shf r3 r3 -20
	shf r3 r3 20
	str r3 r2 0

fb_init_cleanup
	ldr r3 r19 3
	ldr r2 r19 2
	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 4
	jnc r1F r1b 0


uart_init
	ads r18 r18 3
	str r19 r18 -2
	ads r19 r18 -2
	str r1 r19 1
	str r2 r19 2

	xor r1d r1d r1d
	ads r1d r1d 7e40206
	ldr r1 r1d 0
	shf r1 r1 20
	shf r1 r1 -20
	str r1 r1d 0

	ads r1d r1a Mbox
	xor r2 r2 r2
	ads r1 r2 24
	str r1 r1d 0
	ads r1 r2 c
	shf r1 r1 -20
	ads r1 r1 38002
	str r1 r1d 1
	ads r1 r2 2
	shf r1 r1 -20
	ads r1 r1 8
	str r1 r1d 2
	ads r1 r2 3d0900
	str r1 r1d 3
	str r2 r1d 4

	xor r1d r1d r1d
	ads r1d r1d 8
	ads r18 r18 1
	str r1b r18 0
	ads r1b r1e 2
	jnc r1f r1a mbox_call
	ldr r1b r18 0
	ads r18 r18 -1

	ads r1d r2 7e40000
	ldr r1 r1d 0
	ads r2 r2 3f000
	shf r2 r2 -20
	nor r2 r2 r2
	and r1 r1 r2
	xor r2 r2 r2
	ads r2 r2 24000
	shf r2 r2 -20
	orr r1 r1 r2
	str r1 r1d 0

	xor r1 r1 r1
	ads r1d r1 7e40012
	ldr r1 r1d 0
	shf r1 r1 -20
	shf r1 r1 20
	str r1 r1d 0

	xor r1d r1d r1d
	ads r1d r1d a0
	ads r1d r1d -1
	jnc r1d r1e -1

	ads r1 r1d 7e40013
	ldr r1d r1 0
	shf r1d r1d 20
	shf r1d r1d -20
	ads r1d r1d c000
	str r1d r1 0

	xor r1d r1d r1d
	ads r1d r1d a0
	ads r1d r1d -1
	jnc r1d r1e -1

	ldr r1d r1 0
	shf r1d r1d 20
	shf r1d r1d -20
	str r1d r1 0

	xor r1d r1d r1d
	ads r1d r1d 7e40208
	ldr r1 r1d 0
	shf r1 r1 -20
	shf r1 r1 20
	ads r1 r1 7ff00000000
	str r1 r1d 0

	xor r1d r1d r1d
	ads r1d r1d 7e40204
	ldr r1 r1d 0
	shf r1 r1 -20
	shf r1 r1 20
	ads r1 r1 200000000
	str r1 r1d 0

	xor r1d r1d r1d
	xor r1 r1 r1
	ads r1d r1d 7e40205
	ads r1 r1 70
	shf r1 r1 -20
	ads r1 r1 b
	str r1 r1d 0

	xor r1d r1d r1d
	ads r1d r1d 7e40206
	ldr r1 r1d 0
	shf r1 r1 20
	shf r1 r1 -20
	ads r1 r1 301
	str r1 r1d 0

	ldr r2 r19 2
	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 3
	jnc r1F r1b 0


uart_send
	ads r18 r18 3
	str r19 r18 -2
	ads r19 r18 -2
	str r1 r19 1
	str r2 r19 2

	xor r1 r1 r1
	ads r1 r1 7e40203
	ldr r2 r1 0
	and r2 r2 20
	jnc r2 r1e -2

	str r1d r1 -3

	ldr r2 r19 2
	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 3
	jnc r1F r1b 0


uart_getc
	ads r18 r18 3
	str r19 r18 -2
	ads r19 r18 -2
	str r1 r19 1
	str r2 r19 2

	xor r1 r1 r1
	ads r1 r1 7e40203
	ldr r2 r1 0
	and r2 r2 10
	jnc r2 r1e -2
 
	ldr r1d r1 -3
	and r1d r1d FFFFFFFF
	xor r2 r1d d
	jnc r2 r1e 2
	sbs r1d r1d 3

	ldr r2 r19 2
	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 3
	jnc r1F r1b 0


uart_puts
	ads r18 r18 5
	str r19 r18 -4
	ads r19 r18 -4
	str r1 r19 1
	str r2 r19 2
	str r3 r19 3
	str r1b r19 4

	ads r1 r1d 0
	xor r2 r2 r2
	ldr r1d r1 r2
	and r1d r1d FFFF
	xor r3 r1d a
	jnc r3 r1e 2
	ads r1d r1d 3

	ads r3 r1d 0
	ads r1b r1e 2
	jnc r1f r1a uart_send

	ads r2 r2 1
	jnc r3 r1e -9

	ldr r1b r19 4
	ldr r3 r19 3
	ldr r2 r19 2
	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 5
	jnc r1F r1b 0


get_parent
	jnc r1d r1e 2
	jnc r1f r1b 0

	ldr r1d r1d 0
	shf r1d r1d 2
	shf r1d r1d -2
	jnc r1f r1b 0


get_balance_factor
	jnc r1d r1e 3
	ads r1d r1d 3
	jnc r1f r1b 0

	ldr r1d r1d 0
	and r1d r1d 3
	jnc r1f r1b 0


set_balance_factor
	ads r18 r18 2
	str r19 r18 -1
	ads r19 r18 -1
	str r1 r19 1

	ldr r1 r1d 0
	ldr r1d r1d 1
	and r1d r1d 3
	shf r1 r1 2
	shf r1 r1 -2
	orr r1d r1 r1d

	ldr r1 r19 1
	ldr r19 r19 0
	sbs r18 r18 2
	jnc r1F r1b 0


fb_init_args	400
		300
fb_width	0
fb_height	0
fb_pitch	0
fb_isrgb	0
fb_start	0


MAIN
	ads r1c r1a exception_handler

	ads r18 r18 1
	str r1b r18 0
	ads r1b r1e 2
	jnc r1f r1a uart_init
	ldr r1b r18 0
	ads r18 r18 -1

	ads r1d r1a fb_width
	str r1d r1d 0
	ads r1d r1a fb_height
	str r1d r1d 0
	ads r1d r1a fb_pitch
	str r1d r1d 0
	ads r1d r1a fb_isrgb
	str r1d r1d 0
	ads r1d r1a fb_start
	str r1d r1d 0
	ads r1d r1a fb_init_args
	ads r18 r18 1
	str r1b r18 0
	ads r1b r1e 2
	jnc r1f r1a fb_init
	ldr r1b r18 0
	ads r18 r18 -1

	ldr r1d r1a fb_start

	jnc r1f r1b 0

STACK 0
