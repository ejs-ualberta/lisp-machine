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
clang $CFLAGS -c -o kernel.o kernel.c
clang $CFLAGS -c -o data.o gnu-efi*/lib/data.c
clang $LDFLAGS -o BOOTX64.EFI kernel.o data.o
#rm ./kernel.o ./data.o

dd if=/dev/zero of=fat.img bs=1k count=64000
mkfs.vfat fat.img -F 32
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT

mkdir iso
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o os.iso iso
rm -rf ./iso
rm ./fat.img ./BOOTX64.EFI
qemu-system-x86_64 -L /usr/share/ovmf -bios OVMF.fd -cdrom os.iso
