SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Wall -ffreestanding -nostdlib -mcpu=cortex-a53+nosimd -ggdb

default: all
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -dtb "bcm2710-rpi-3-b-plus.dtb" -serial stdio #-S -gdb "tcp::1236" #& #-d in_asm
	#gdb-multiarch -x .gdbinit
all: khead kernel8.img clean
khead: kernel
	dd if=/dev/zero bs=1 count=1 >> ./kernel
	xxd -i ./kernel > ./kc_ascii.h
	tr < kernel -d '\000' > kernel.tmp
	mv ./kernel.tmp ./kernel
kernel8.img: kernel boot.S $(OBJS)
	clang --target=aarch64-elf $(CFLAGS) -c boot.S -o boot.o
	ld.lld -m aarch64elf -nostdlib boot.o $(OBJS) -T link.ld -o kernel8.elf
	llvm-objcopy -O binary kernel8.elf kernel8.img
%.o: %.c
	clang --target=aarch64-elf $(CFLAGS) -c $< -o $@
clean:
	rm *.o>/dev/null 2>/dev/null || true
