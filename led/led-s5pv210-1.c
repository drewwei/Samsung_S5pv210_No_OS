#include <linux/module.h>		// module_init  module_exit
#include <linux/init.h>			// __init   __exit
#include <linux/fs.h>
#include <linux/leds.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <mach/gpio.h>

#define gpj0(_nr)		S5PV210_GPJ0(_nr)   //获取gpio号

//#define GPJ0CON  S5PV210_GPJ0CON  //静态映射
//#define GPJ0DAT  S5PV210_GPJ0DAT

static struct led_classdev *led1;
static struct led_classdev *led2;
static struct led_classdev *led3;

static void led1_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	
	printk(KERN_INFO "s5pv210_led1_set\n");
	if(value==LED_OFF)
	{
		//writel(readl(GPJ0DAT)|(1<<3), GPJ0DAT);
		gpio_set_value(gpj0(3),1);
	}else
	{
		//writel(readl(GPJ0DAT)&~(1<<3), GPJ0DAT);
		gpio_set_value(gpj0(3),0);
		
	}
}
static void led2_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	printk(KERN_INFO "s5pv210_led2_set\n");
	if(value==LED_OFF)
	{
		//writel(readl(GPJ0DAT)|(1<<4), GPJ0DAT);
		gpio_set_value(gpj0(4),1);
	}
	if(value==LED_FULL)
	{
		//writel(readl(GPJ0DAT)&~(1<<4), GPJ0DAT);
		gpio_set_value(gpj0(4),0);
	}
}
static void led3_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	printk(KERN_INFO "s5pv210_led3_set\n");
	if(value==LED_OFF)
	{
		//writel(readl(GPJ0DAT)|(1<<5), GPJ0DAT);
		gpio_set_value(gpj0(5),1);
	}
	if(value==LED_FULL)
	{
		//writel(readl(GPJ0DAT)&~(1<<5), GPJ0DAT);
		gpio_set_value(gpj0(5),0);
	}
}


static int __init s5pv21_led_init(void)
{
	if (gpio_request(gpj0(3),"led3_gpio")) 
	{
		printk(KERN_INFO "gpio_request fail.\n");
		return -1;
	}
	gpio_direction_output(gpj0(3),1);
	if (gpio_request(gpj0(4),"led4_gpio")) 
	{
		printk(KERN_INFO "gpio_request fail.\n");
		return -1;
	}
	gpio_direction_output(gpj0(4),1);
	if (gpio_request(gpj0(5),"led5_gpio")) 
	{
		printk(KERN_INFO "gpio_request fail.\n");
		return -1;
	}
	gpio_direction_output(gpj0(5),1);
	printk(KERN_INFO "s5pv21_led_init\n");
	led1=kzalloc(sizeof(struct led_classdev),GFP_KERNEL);
	if(!led1)
	{
		printk("kzalloc led1 error.\n");
		return -1;
	}
	led2=kzalloc(sizeof(struct led_classdev),GFP_KERNEL);
	if(!led2)
	{
		printk("kzalloc led2 error.\n");
		return -1;
	}
	led3=kzalloc(sizeof(struct led_classdev),GFP_KERNEL);
	if(!led3)
	{
		printk("kzalloc led3 error.\n");
		return -1;
	}
	led1->name="led1";
	led1->brightness=0;
	led1->brightness_set=led1_brightness_set;
	if(led_classdev_register(NULL, led1))
	{
		return -1;
	}
	
	led2->name="led2";
	led2->brightness=0;
	led2->brightness_set=led2_brightness_set;
	if(led_classdev_register(NULL, led2))
	{
		return -1;
	}
	led3->name="led3";
	led3->brightness=0;
	led3->brightness_set=led3_brightness_set;
	if(led_classdev_register(NULL, led3) < 0)
	{
		return -1;
	}
	
	return 0;
}

static void __exit s5pv21_led_exit(void)
{
	led_classdev_unregister(led1);
	led_classdev_unregister(led2);
	led_classdev_unregister(led3);
	gpio_free(gpj0(3));
	gpio_free(gpj0(4));
	gpio_free(gpj0(5));
	kfree(led1);
	kfree(led2);
	kfree(led3);
}

module_init(s5pv21_led_init);
module_exit(s5pv21_led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("DREW");
MODULE_DESCRIPTION("MY_MODULES");