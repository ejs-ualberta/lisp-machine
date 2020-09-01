.global irq0
 
.global load_idt
 
irq0:
  ret
 
load_idt:
	mov 4(%esp), %edx
	lidt (%edx)
	sti
	ret
