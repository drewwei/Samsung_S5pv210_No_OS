
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>


static int x210_attach (struct i2c_adapter * adapter)
{
	printk("x210_attach");
	return 0;
}

static int x210_detach (struct i2c_adapter *)
{
	printk("x210_detach");
	return 0;
}
	

static struct i2c_driver x210_i2c_driver = {
	.driver = {
		.name	    = "x210_i2c",
	},
	.attach_adapter  = x210_attach,
	.detach_adapter = x210_detach,
};


static int __init x210_i2c_init(void)
{
	return i2c_add_driver(&x210_i2c_driver);
}

static void __init x210_i2c_exit()
{
	i2c_del_driver(&x210_i2c_driver);
}

module_init(x210_i2c_init);
module_exit(x210_i2c_exit);


MODULE_LICENSE("GPL");







































