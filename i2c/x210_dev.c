#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>

static struct i2c_client *client;

static struct i2c_board_info x210_info = {
	I2C_BOARD_INFO("x210_i2c", 0x50),
};


static int __init x210_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	i2c_adap = i2c_get_adapter(2);
	client=i2c_new_device(i2c_adap, &x210_info);
	return 0;
}

static void __exit x210_dev_exit(void)
{
	i2c_unregister_device(client);
}


module_init(x210_dev_init);
module_exit(x210_dev_exit);

MODULE_LICENSE("GPL");

