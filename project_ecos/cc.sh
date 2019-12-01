source /opt/ecos/ecosenv.sh
make
i386-elf-objcopy -O binary program program.bin
sudo dd conv=sync if=program.bin of=/dev/fd0
