KERN_DIR = /root/kernel
obj-m  += key_x210.o
all:
	make -C $(KERN_DIR) M=`pwd` modules
cp:
	cp *.out *.ko /root/porting_x210/rootfs/rootfs/root -f
PHONY:clean
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
