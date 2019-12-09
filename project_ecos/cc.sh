source /opt/ecos/ecosenv.sh
make
i386-elf-objcopy -O binary ecos ecos.bin
sudo dd conv=sync if=ecos.bin of=/dev/fd0
