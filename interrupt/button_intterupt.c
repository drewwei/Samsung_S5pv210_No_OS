#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/wait.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>

#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <mach/gpio.h>

#include <plat/map-base.h>


#define irq           IRQ_EINT2
#define GPH0_2   S5PV210_GPH0(2)
//static irq_handler_t  button_handler;
static dev_t num;
static struct cdev * dev;

static int ev_press=0 ;

static DECLARE_WAIT_QUEUE_HEAD (button_wait);

static irqreturn_t button_interrupt(int irq, void *dummy)
{
	ev_press=1;
	wake_up_interruptible(&button_wait);
	
	return IRQ_HANDLED;
}

static int button_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	int ret;
	int value;
	/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(button_wait, ev_press);
	ev_press=0;
	ret=gpio_get_value(GPH0_2);
	if(ret)
		{
			value=0;
		}
	else{
			value=1;
		}
	if (copy_to_user(buff, &value, sizeof(value)));
			return -1;
	return 0;	
}

static int button_open(struct inode *inode, struct file *file)
{
//申请和注册中断
	int ret;
	ret=request_irq(irq, button_interrupt, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING , "button_interrupt", NULL);
	if(!ret)
		{
			printk(KERN_ERR "request_irq fali.\n");
			return -1;
		}
	
 return 0;
}




static struct file_operations fops={
	.owner=THIS_MODULE,
	.open=button_open,
	.read=button_read,
};

static int __init button_init(void)
{
	int ret=-1;
	ret= alloc_chrdev_region(&num,0,1,"button_interrupt");
	if(ret<0)
		{
			printk(KERN_INFO "alloc_chrdev_region fail.\n");
			return -1;
		}
	dev=cdev_alloc();
	if(!dev)
		{
			printk(KERN_INFO "cdev_alloc fail.\n");
			goto alloc_chrdev_region_error;
		}
	dev->owner = fops.owner;
 	dev->ops = &fops;
	cdev_add(dev,num,1);
	
	return 0;
alloc_chrdev_region_error:
	unregister_chrdev_region(num, 1);
return -1;
	
}


static void __exit button_exit(void)
{
	unregister_chrdev_region(num, 1);
	cdev_del(dev);
	free_irq(irq, NULL);
}


module_init(button_init);
module_exit(button_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DREW 1677812132@qq.com");
MODULE_DESCRIPTION("MY_MODULES");
