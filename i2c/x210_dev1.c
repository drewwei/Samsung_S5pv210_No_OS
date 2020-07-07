#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>

static struct i2c_client *client;

static const unsigned short addr_list[] = { 0x0f, I2C_CLIENT_END };

static int __init x210_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info i2c_info;
	
	i2c_adap = i2c_get_adapter(0);
	memset(&i2c_info, 0, sizeof(struct i2c_board_info));	
	strlcpy(i2c_info.type, "x210_i2c", I2C_NAME_SIZE);
	client=i2c_new_probed_device(i2c_adap, &i2c_info, addr_list);
	i2c_put_adapter(i2c_adap);
	return 0;
}

static void __exit x210_dev_exit(void)
{
	i2c_unregister_device(client);
}


module_init(x210_dev_init);
module_exit(x210_dev_exit);

MODULE_LICENSE("GPL");

