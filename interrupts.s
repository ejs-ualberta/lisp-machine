.set IRQ_BASE, 0x20
.global int_routine
.section .text

.extern interrupt_handler

int_routine:
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax

    call interrupt_handler

    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rbp

.global IgnoreInterrupt
IgnoreInterrupt:

    iretq
