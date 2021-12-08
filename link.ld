SECTIONS
{
    . = 0x80000;
    .text : { KEEP(*(.text.boot)) *(.text) }
    .rodata : { *(.rodata) }
    . = ALIGN(4096);
    .data : { *(.data) }
    . = ALIGN(16);
    __bss_start = .;
    .bss (NOLOAD) : { *(.bss) }
    __bss_end = .;
    . = ALIGN(4096);
    _end = .;
}
__bss_size = (__bss_end - __bss_start) >> 3;
