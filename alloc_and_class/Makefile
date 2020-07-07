KERN_DIR = /root/kernel
obj-m	+= test.o
#obj-m	+= module_test.o

all:
	#make -C $(KERN_DIR) M=`pwd` modules
	make -C $(KERN_DIR) M=`pwd` modules 
cp:
	cp *.ko app /root/porting_x210/rootfs/rootfs/root -f
.PHONY:clean
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean