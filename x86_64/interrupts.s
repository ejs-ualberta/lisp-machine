.global int_routine
.section .text

.extern interrupt_handler

.macro pushaq
push %rax
push %rbx
push %rcx
push %rdx
push %rbp
push %rdi
push %rsi
push %r8
push %r9
push %r10
push %r11
push %r12
push %r13
push %r14
push %r15
.endmacro

.macro popaq
pop %r15
pop %r14
pop %r13
pop %r12
pop %r11
pop %r10
pop %r9
pop %r8
pop %rsi
pop %rdi
pop %rbp
pop %rdx
pop %rcx
pop %rbx
pop %rax
.endmacro

.macro EXC num
.global EXC_\num
EXC_\num:
    movb $\num, (irqnum)
    jmp int_routine

.endm

.macro IRQ num
.global IRQ_\num
IRQ_\num:
    movb $\num, (irqnum)
    jmp int_routine

.endm

int_routine:
    pushaq	
    mov (irqnum), %sil
    call interrupt_handler	
    popaq
    iretq

IRQ 0

.data
irqnum: .byte 0
