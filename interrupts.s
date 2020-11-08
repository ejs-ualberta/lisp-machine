.global int_routine
.section .text

.extern interrupt_handler

int_routine:
    call interrupt_handler
    iretq
