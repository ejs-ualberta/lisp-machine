set pagination off
set history save on

define hook-next
  info locals
end

define hook-step
  info locals
end

file kernel8.elf
target remote localhost:1236
break main
continue
