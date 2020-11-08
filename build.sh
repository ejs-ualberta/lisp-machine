CFLAGS='-target x86_64-unknown-windows 
        -ffreestanding 
        -fshort-wchar 
        -mno-red-zone 
        -I./gnu-efi/inc -I./gnu-efi/inc/x86_64 -I./gnu-efi/inc/protocol'
LDFLAGS='-target x86_64-unknown-windows 
        -nostdlib 
        -Wl,-entry:efi_main 
        -Wl,-subsystem:efi_application 
        -fuse-ld=lld-link'

clang $CFLAGS -c -o debug.o debug.c
clang $CFLAGS -c -o util.o util.c
clang $CFLAGS -c -o datastructures.o datastructures.c
clang $CFLAGS -c -o asm.o asm.c
clang $CFLAGS -c -o alloc.o alloc.c
clang $CFLAGS -c -o kernel.o kernel.c
clang $CFLAGS -c -o data.o gnu-efi*/lib/data.c
clang $CFLAGS -c -o ints.o interrupts.s
clang $LDFLAGS -o BOOTX64.EFI ./*.o
rm ./*.o

dd if=/dev/zero of=fat.img bs=1k count=64000
mkfs.vfat fat.img -F 32
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT
mcopy -i fat.img kernel ::/kernel

mkdir iso
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o os.iso iso
rm -rf ./iso
rm ./fat.img ./BOOTX64.EFI
qemu-system-x86_64 -L /usr/share/ovmf -bios OVMF.fd -cdrom os.iso -m 1024M -d int -monitor stdio #-full-screen -device VGA,edid=on,xres=1366,yres=768 # (For when qemu has edid support)
