source /opt/ecos/ecosenv.sh
make
i386-elf-objcopy -O binary ecos ecos.bin
sudo dd conv=sync if=ecos.bin of=/dev/fd0
sudo qemu-system-i386 -fda ecos.bin -serial /dev/ttyUSB0
