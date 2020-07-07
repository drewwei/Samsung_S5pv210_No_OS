KERN_DIR = /root/kernel
obj-m  += x210_drv.o
obj-m  += x210_dev1.o
all:
	make -C $(KERN_DIR) M=`pwd` modules
cp:
	cp app *.out *.ko /root/porting_x210/rootfs/rootfs/root -f
PHONY:clean
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
