set pagination off
set history save on

file kernel8.elf
target remote localhost:1236
break breakp
break main
continue
