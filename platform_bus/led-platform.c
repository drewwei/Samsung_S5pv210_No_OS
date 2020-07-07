#include <linux/module.h>		// module_init  module_exit
#include <linux/init.h>			// __init   __exit
#include <linux/fs.h>
#include <linux/leds.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <mach/leds-gpio.h>
#include <linux/slab.h>

//static unsigned int		 gpio_num;
//#define gpj0(_nr)		S5PV210_GPJ0(_nr)   //获取gpio号

//#define GPJ0CON  S5PV210_GPJ0CON  //静态映射
//#define GPJ0DAT  S5PV210_GPJ0DAT


struct s5pv210_gpio_led {
	struct led_classdev		 cdev;
	struct s5pv210_led_platdata	*pdata;
	};


static inline struct s5pv210_gpio_led *pdev_to_gpio(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}


static void s5pv210_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct s5pv210_gpio_led *p = container_of(led_cdev,struct s5pv210_gpio_led,cdev);
	printk(KERN_INFO "s5pv210_led1_set\n");
	if(value==LED_OFF)
	{
		//writel(readl(GPJ0DAT)|(1<<3), GPJ0DAT);
		gpio_set_value(p->pdata->gpio,1);
	}else
	{
		//writel(readl(GPJ0DAT)&~(1<<3), GPJ0DAT);
		gpio_set_value(p->pdata->gpio,0);
	}
}
static int s5pv210_led_probe(struct platform_device *dev)
{
	int ret= -1;
	//printk(KERN_INFO "s5pv210_led_probe\n");
	struct s5pv210_led_platdata *pdata = dev->dev.platform_data;
	struct s5pv210_gpio_led *led;
	led = kzalloc(sizeof(struct s5pv210_gpio_led), GFP_KERNEL);
	if (led == NULL) 
	{
		printk(KERN_INFO "No memory for device\n");
		return ret;
	}
	platform_set_drvdata(dev, led);

	// 在这里去申请驱动用到的各种资源，当前驱动中就是GPIO资源
	if (gpio_request(pdata->gpio, pdata->name)) 
	{
		printk(KERN_ERR "gpio_request failed\n");
	} 
	else 
	{
		// 设置为输出模式，并且默认输出1让LED灯灭
		gpio_direction_output(pdata->gpio, 1);
	}
	
	led->cdev.brightness_set =s5pv210_led_set;
	led->cdev.brightness = 0;
	led->cdev.name = pdata->name;
	led->pdata = pdata;
	
	//gpio_num=pdata->gpio;
	
	ret = led_classdev_register(&dev->dev, &led->cdev);
	if (ret < 0) {
		printk(KERN_ERR "led_classdev_register failed\n");
		kfree(led);
		return ret;
	}
	return 0;
}
static int s5pv210_led_remove(struct platform_device *dev)
{
	struct s5pv210_gpio_led *led = pdev_to_gpio(dev);
	struct s5pv210_led_platdata *pdata = dev->dev.platform_data;
	led_classdev_unregister(&led->cdev);
	kfree(led);
	gpio_free(pdata->gpio);
	return 0;
}

static struct platform_driver s5pv210_led_driver = {
	.probe		= s5pv210_led_probe,
	.remove		= s5pv210_led_remove,
	.driver		= {
		.name		= "s5pv210-led",
		.owner		= THIS_MODULE,
	},
};
static int __init s5pv21_led_init(void)
{
	return platform_driver_register(&s5pv210_led_driver);
}

static void __exit s5pv21_led_exit(void)
{
	platform_driver_unregister(&s5pv210_led_driver);
}

module_init(s5pv21_led_init);
module_exit(s5pv21_led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("DREW 1677812132@qq.com");
MODULE_DESCRIPTION("MY_MODULES");