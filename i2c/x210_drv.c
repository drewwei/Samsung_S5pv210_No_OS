#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static struct class *cls;
static dev_t num;
static struct cdev *cdev;
static struct i2c_client *cli;
/*
ssize_t i2c_read (struct file *file, char __user *buf, size_t size, loff_t *off)
{
	int value, addr;
	if(copy_from_user(&addr, buf, size)) 
		return -EIO;
	value = i2c_smbus_read_byte_data(cli, addr);
	printk("i2c_read:%d\n",value);
	value = (value&0xfc)>>2;
	if(copy_to_user(buf, &value, size))
		return -EIO;
	return 1;
}
*/
ssize_t i2c_read (struct file *file, char __user *buf, size_t size, loff_t *off)
{
	unsigned char addr;
	unsigned char data;
	struct i2c_msg msg[2];
	int ret;

	if(size != 1)
		return -EINVAL;	
	if(copy_from_user(&addr, buf, 1))
		return -EIO;

	msg[0].addr = cli->addr;
	msg[0].buf = &addr;
	msg[0].len = 1;
	msg[0].flags = 0;
	
	msg[1].addr = cli->addr;
	msg[1].buf = &data;
	msg[1].len = 1;
	msg[1].flags = I2C_M_RD;

	ret = i2c_transfer(cli->adapter, msg, 2);
	if(ret ==2)
		{
			if(copy_to_user(buf, &data, 1))
				return -EIO;
			//printk("data1:%d\n",data);
			return 1;
		}
	else
		return -EIO;	

}

static ssize_t i2c_write (struct file *file, const char __user *buf, size_t cout, loff_t *off)
{
	int ret;
	unsigned char kbuf[2];
	copy_from_user(kbuf, buf, 2);
	ret = i2c_smbus_write_byte_data(cli, kbuf[0], kbuf[1]);
	if(ret)
		return -EIO;
	return 0;
}


static int i2c_open(struct inode * inode, struct file * file)
{
   
     return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open   = i2c_open,
	.read   = i2c_read,
	.write   = i2c_write,
};


static int x210_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	printk("x210_probe\n");
	cli = client;
	 ret = i2c_smbus_read_byte_data(client, 0x12);
	 printk("XOUT:%d",ret);
	ret = alloc_chrdev_region(&num, 0, 1, "x210_i2c");
	if(ret){
			printk("alloc_chrdev_region fail.\n");
			return -ENODEV;
		}
	cdev = cdev_alloc();
	if(!cdev)
	{
		printk("cdev_alloc fail.\n");
		unregister_chrdev_region(num, 1);
		return -ENODEV;
	}
	cdev->owner = THIS_MODULE;
	cdev->ops = &fops;
	cdev_add(cdev, num, 1);

	cls = class_create(THIS_MODULE, "MY_I2C");
	device_create(cls, NULL, num, NULL, "my_i2c");
	return 0;
}
static int x210_remove(struct i2c_client *client)
{
	printk("x210_remove");
	return 0;
}

static const struct i2c_device_id x210_id[] = {
	{ "x210_i2c", 0 },
	{ }
};


static struct i2c_driver x210_drv = {
	.driver = {
		.name	= "x210",
	},
	.probe		= x210_probe,
	.remove		= x210_remove,
	.id_table	=x210_id ,

};


static int __init x210_drv_init(void)
{
	return i2c_add_driver(&x210_drv);
}

static void __exit x210_drv_exit(void)
{
	i2c_del_driver(&x210_drv);
	unregister_chrdev_region(num, 1);
	cdev_del(cdev);
	device_destroy(cls, num);
	class_destroy(cls);
	
}


module_init(x210_drv_init);
module_exit(x210_drv_exit);

MODULE_LICENSE("GPL");

