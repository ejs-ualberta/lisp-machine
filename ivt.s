	.global i_handler
	.global load_idt
	.extern interrupt_handler

load_idt:
	mov 4(esp), edx
	lidt (edx)
	sti
	ret

i_handler:
	pusha
	call interrupt_handler
	popa
	iret
