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

#define GPJ0CON  S5PV210_GPJ0CON  //静态映射
#define GPJ0DAT  S5PV210_GPJ0DAT

static struct led_classdev *led1;
static struct led_classdev *led2;
static struct led_classdev *led3;

static void led1_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	
	printk(KERN_INFO "s5pv210_led1_set\n");
	if(value==LED_OFF)
	{
		writel(readl(GPJ0DAT)|(1<<3), GPJ0DAT);
	}
	if(value==LED_FULL)
	{
		writel(readl(GPJ0DAT)&~(1<<3), GPJ0DAT);
	}
}
static void led2_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	printk(KERN_INFO "s5pv210_led2_set\n");
	if(value==LED_OFF)
	{
		writel(readl(GPJ0DAT)|(1<<4), GPJ0DAT);
	}
	if(value==LED_FULL)
	{
		writel(readl(GPJ0DAT)&~(1<<4), GPJ0DAT);
	}
}
static void led3_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	printk(KERN_INFO "s5pv210_led3_set\n");
	if(value==LED_OFF)
	{
		writel(readl(GPJ0DAT)|(1<<5), GPJ0DAT);
	}
	if(value==LED_FULL)
	{
		writel(readl(GPJ0DAT)&~(1<<5), GPJ0DAT);
	}
}


static int __init s5pv21_led_init(void)
{
	int i;
	writel(0x11111111, GPJ0CON);
	for(i=0;i<2;i++)
	{
		writel((0<<3)|(0<<4)|(0<<5), GPJ0DAT);
		mdelay(500);
		writel((1<<3)|(1<<4)|(1<<5), GPJ0DAT);
		mdelay(500);
	}
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
	led1->brightness=255;
	led1->brightness_set=led1_brightness_set;
	if(led_classdev_register(NULL, led1))
	{
		return -1;
	}
	
	led2->name="led2";
	led2->brightness=255;
	led2->brightness_set=led2_brightness_set;
	if(led_classdev_register(NULL, led2))
	{
		return -1;
	}
	led3->name="led3";
	led3->brightness=255;
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
	kfree(led1);
	kfree(led2);
	kfree(led3);
}

module_init(s5pv21_led_init);
module_exit(s5pv21_led_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("DREW");
MODULE_DESCRIPTION("MY_MODULES");