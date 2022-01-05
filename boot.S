#define BUF_SZ 512
//TODO: what if large amount of stuff on stack but sp not incremented yet, then interrupt? Make label for register storage? But then no nested interrupts, but didn't really want those anyway.
	
    .section ".text.boot"	
    .global _start

    .macro  kernel_entry
    sub     sp, sp, #BUF_SZ
    stp     x0, x1, [sp, #16 * 0]
    stp     x2, x3, [sp, #16 * 1]
    stp     x4, x5, [sp, #16 * 2]
    stp     x6, x7, [sp, #16 * 3]
    stp     x8, x9, [sp, #16 * 4]
    stp     x10, x11, [sp, #16 * 5]
    stp     x12, x13, [sp, #16 * 6]
    stp     x14, x15, [sp, #16 * 7]
    stp     x16, x17, [sp, #16 * 8]
    stp     x18, x19, [sp, #16 * 9]
    stp     x20, x21, [sp, #16 * 10]
    stp     x22, x23, [sp, #16 * 11]
    stp     x24, x25, [sp, #16 * 12]
    stp     x26, x27, [sp, #16 * 13]
    stp     x28, x29, [sp, #16 * 14]
    stp     x30, x31, [sp, #16 * 15] 
    .endm

    .macro  kernel_exit
    ldp     x0, x1, [sp, #16 * 0]
    ldp     x2, x3, [sp, #16 * 1]
    ldp     x4, x5, [sp, #16 * 2]
    ldp     x6, x7, [sp, #16 * 3]
    ldp     x8, x9, [sp, #16 * 4]
    ldp     x10, x11, [sp, #16 * 5]
    ldp     x12, x13, [sp, #16 * 6]
    ldp     x14, x15, [sp, #16 * 7]
    ldp     x16, x17, [sp, #16 * 8]
    ldp     x18, x19, [sp, #16 * 9]
    ldp     x20, x21, [sp, #16 * 10]
    ldp     x22, x23, [sp, #16 * 11]
    ldp     x24, x25, [sp, #16 * 12]
    ldp     x26, x27, [sp, #16 * 13]
    ldp     x28, x29, [sp, #16 * 14]
    ldp     x30, x31, [sp, #16 * 15] 
    add     sp, sp, #BUF_SZ
    eret
    .endm	

_start:
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
1:  wfe
    b       1b
2:  ldr     x1, =_start

    // Qemu boots in EL2, so if booting on real hardware only Section 1 can be removed.
    // Section 1 Start
    mov     x3, x0
    mrs     x0, CurrentEL
    and     x0, x0, #12
    cmp     x0, #4
    beq     5f

    msr     sp_el1, x1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret
5:  mov     x0, x3
    // Section 1 End	

    mov     sp, x1
    ldr     x2, =ivt
    msr     vbar_el1, x2

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size
3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

4:  bl      main
    b       1b


    .align  11
ivt:
    .align  7
    b       sync_handler

    .align  7
    b       irq_handler

    .align  7
    b       fiq_handler

    .align  7
    b       error_handler	


irq_handler:
    kernel_entry
    mov     x0, #1
    b       common_handler


fiq_handler:
    kernel_entry
    mov     x0, #2
    b       common_handler


error_handler:
    kernel_entry
    mov     x0, #3
    b       common_handler


sync_handler:
    kernel_entry
    mov     x0, #0


common_handler:
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, spsr_el1
    mrs     x4, far_el1
    bl      exc_handler
    kernel_exit
