.set IRQ_BASE, 0x20
.global int_routine
.section .text

.extern gdt_ptr
gdt_flush:
    lgdt [gdt_ptr]
    mov ax, 0x10
    mov ss, ax
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax
    push 0x08
    push .farjmp
    o64 retf
.farjmp:
    ret

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
