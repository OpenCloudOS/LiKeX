make
insmod inspector.ko
rmmod inspector
make clean
dmesg | grep cpu
