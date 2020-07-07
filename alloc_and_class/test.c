#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/device.h>
//#include <asm-genric/io.h> //静态映射头文件

#define MYCNT  	 1  
//#define GPJ0CON  S5PV210_GPJ0CON  //静态映射
//#define GPJ0DAT  S5PV210_GPJ0DAT
#define GPJ0BASE 0xE0200240
#define S5PV210(x)      (x)
#define GPJ0CON_REG     S5PV210(0)
#define GPJ0DAT_REG	    S5PV210(4)
//#define PGPJ0CON   (regs+GPJ0CON_REG)
//#define PGPJ0DAT   (regs+GPJ0DAT_REG)
#define mydev	 "test"
typedef struct s5p
{
	volatile unsigned int pGPJ0CON;
	volatile unsigned int pGPJ0DAT;
}struct_s5p;
struct_s5p __iomem *ps5p;
char kbuf[100];
static struct cdev *pcdev;
static dev_t dev;
static struct class *myclass;
static struct device *device;

static ssize_t module_write(struct file *file, const char __user *buf,size_t count, loff_t *ppos)
{
	printk(KERN_INFO "test_chrdev_write\n");
	memset(kbuf,0,sizeof(kbuf));
	if(copy_from_user(kbuf,buf,4))
	{
		return -1;
	}
	if(kbuf[0]=='1')//写1亮
	{
		//writel((0<3)|(0<4)|(0<5),&(ps5p->pGPJ0DAT));
		ps5p->pGPJ0DAT=(0<3)|(0<4)|(0<5);
	}
	else if(kbuf[0]=='0')
	{
		//writel((1<<3) | (1<<4) | (1<<5),&(ps5p->pGPJ0DAT));
		ps5p->pGPJ0DAT=(1<<3)|(1<<4)|(1<<5);
	}
	return 0;
}
static ssize_t module_read(struct file *file, __user char *buffer, size_t count,loff_t *ppos)
{

	int ret = -1;
	
	printk(KERN_INFO "test_chrdev_read\n");
	
	ret = copy_to_user(buffer, kbuf, count);
	if (ret)
	{
		printk(KERN_ERR "copy_to_user fail\n");
		return -EINVAL;
	}
	printk(KERN_INFO "copy_to_user success..\n");	
	return 0;
}

static int module_open(struct inode *inode, struct file *file)
{
	
	printk(KERN_INFO "test open sucess.\n");
	//writel(0x11111111,&(ps5p->pGPJ0CON));
	ps5p->pGPJ0CON=0x11111111;
	//writel((0<3)|(0<4)|(0<5),&(ps5p->pGPJ0DAT));
	ps5p->pGPJ0DAT=(0<3)|(0<4)|(0<5);
	return 0;
}
static int module_close(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "test_chrdev_release\n");
	//writel((1<<3) | (1<<4) | (1<<5),&(ps5p->pGPJ0DAT));
	ps5p->pGPJ0DAT=(1<<3)|(1<<4)|(1<<5);
	return 0;
}

static const struct file_operations fops={
	.owner    =THIS_MODULE,
	.read     =module_read,
	.write    =module_write,
	.open     =module_open,
	.release  =module_close,
};

static int __init mymodule_init(void)
{
	int retval;
	int mymajor;
	//注册设备号
	/*
	retval = register_chrdev_region(MKDEV(MYMAJ, 0),MYCNT, mydev);
	if (retval) 
	{
		printk("Unable to register major %s.\n",mydev);
		return -1;
	}
	*/
//自动分配字符设备号
	retval = alloc_chrdev_region(&dev, 0, MYCNT, mydev);
	if (retval)
	{
		printk(KERN_ERR "alloc_chrdev_region fail.\n");
		return -1;
	}
	mymajor = MAJOR(dev);
//创建设备类
	myclass = class_create(THIS_MODULE, "drewclass");
	if (IS_ERR(myclass)) 
	{
		printk(KERN_ERR "class_create failed\n");
		goto unregister_chrdev_region_err;
	}
//自动创建字符设备文件
	device = device_create(myclass,NULL,dev,NULL,mydev);
	if (IS_ERR(device)) 
	{
		printk(KERN_ERR "device_create failed\n");
		goto class_destroy_err;
	}
//动态申请cdev结构体内存,将owner和file_operations结构体绑定（赋值）到cdev结构体变量中，即注册。
	pcdev = cdev_alloc();
	if (!pcdev) 
	{
		printk(KERN_ERR "cdev_alloc failed\n");
		goto device_del_err;
	}
	pcdev->owner = THIS_MODULE;
	pcdev->ops = &fops;
/*
	cdev_init(&cdev, &fops);  //绑定结构体变量cdev和file_operations 结构体变量
*/
	//通过cdev_add函数注册驱动，即将cdev绑定到内核链表中。即真正的注册驱动。
	if ((retval = cdev_add(pcdev, dev, MYCNT)) != 0)
		{
		printk(KERN_ERR "unable register character device\n");
		goto cdev_del_err;
		}
	if (!request_mem_region(GPJ0BASE,sizeof(*ps5p),"GPJ0"))
	{
		printk(KERN_ERR "failed to request_mem_region!\n");
		goto cdev_del_err;
	}
	ps5p = ioremap(GPJ0BASE,sizeof(*ps5p));
	if (!ps5p)
	{
		printk(KERN_ERR "failed to ioremap regs!\n");
		goto release_mem_region_err;
	}
	printk(KERN_INFO "insmod sucess.\n");
	return 0;
release_mem_region_err:
	release_mem_region(GPJ0BASE, sizeof(*ps5p));
cdev_del_err:
	cdev_del(pcdev);
	printk(KERN_INFO "cdev_del.\n");
device_del_err:
	device_del(device);
	printk(KERN_INFO "device_del.\n");
class_destroy_err :
	class_destroy(myclass);
	printk(KERN_INFO "class_destroy.\n");
unregister_chrdev_region_err :
	unregister_chrdev_region(dev, MYCNT);	
	printk(KERN_INFO "unregister_chrdev_region.\n");
	return -1;
}

static void __exit mymodule_cleanup(void)

{
	printk(KERN_INFO"rmmod sucess.\n");
	device_del(device);
	class_destroy(myclass);
	cdev_del(pcdev);
	unregister_chrdev_region(dev, MYCNT);
	iounmap(ps5p);
	release_mem_region(GPJ0BASE, sizeof(*ps5p));
}


module_init(mymodule_init);
module_exit(mymodule_cleanup);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("DREW");
MODULE_DESCRIPTION("MY_MODULES");