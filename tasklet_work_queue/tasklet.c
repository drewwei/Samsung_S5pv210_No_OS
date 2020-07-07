#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include <asm/irq.h>
#include <asm/io.h>

#include <mach/gpio.h>
#include <plat/map-base.h>
/*
 * X210:
 *
 * POWER  -> EINT1   -> GPH0_1
 * LEFT   -> EINT2   -> GPH0_2
 * DOWN   -> EINT3   -> GPH0_3
 * UP     -> KP_COL0 -> GPH2_0
 * RIGHT  -> KP_COL1 -> GPH2_1
 * MENU   -> KP_COL3 -> GPH2_3 (KEY_A)
 * BACK   -> KP_COL2 -> GPH2_2 (KEY_B)
 */
#define  BUTTON_IRQ    IRQ_EINT2
#define LEFT 	  KEY_LEFT

static struct input_dev *button_dev;

static void func(unsigned long data)
{
	int value;
	s3c_gpio_cfgpin(S5PV210_GPH0(2), S3C_GPIO_SFN(0));
	value=gpio_get_value(S5PV210_GPH0(2));
	s3c_gpio_cfgpin(S5PV210_GPH0(2), S3C_GPIO_SFN(0xf));
	input_report_key(button_dev, LEFT,!!value);
	input_sync(button_dev);   //产生一个同步信号
}

static DECLARE_TASKLET(mytasklet, func, 0);

static irqreturn_t button_interrupt(int irq, void *dummy)
{
	printk(KERN_INFO "This is top interrupt.\n");
	tasklet_schedule(&mytasklet);
	return IRQ_HANDLED;
}

static int __init button_init(void)
{
	int error;
	error=gpio_request(S5PV210_GPH0(2), "GPH0_2");
	if(error)
		printk("button-x210: request gpio GPH0(2) fail");
	s3c_gpio_cfgpin(S5PV210_GPH0(2), S3C_GPIO_SFN(0xf));

	if (request_irq(BUTTON_IRQ, button_interrupt, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, "button", NULL)) {
                printk(KERN_ERR "button.c: Can't allocate irq %d\n", BUTTON_IRQ);
                return -EBUSY;
        }

	button_dev = input_allocate_device();  //分配一个input_dev结构体
	if (!button_dev) {
		printk(KERN_ERR "button.c: Not enough memory\n");
		error = -ENOMEM;
		goto err_free_irq;
	}

	//button_dev->evbit[0] = BIT_MASK(EV_KEY);
	//button_dev->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0);
	set_bit(EV_KEY, button_dev->evbit);  //设置bit位  让input_dev结构体下的evbit[0]=1<<EV_KEY
	set_bit(LEFT, button_dev->keybit);  //addr[nr / BITS_PER_LONG] |= 1UL << (nr % BITS_PER_LONG);
	button_dev->name = "LEFFT_KEY";

	error = input_register_device(button_dev); //用input子系统驱动框架提供的接口函数，向内核注册input_dev
	if (error) {
		printk(KERN_ERR "button.c: Failed to register device\n");
		goto err_free_dev;
	}

	return 0;

 err_free_dev:
	input_free_device(button_dev);  //注销input_dev设备
 err_free_irq:
	free_irq(BUTTON_IRQ, NULL);   //注销终端号。
	return error;
}

static void __exit button_exit(void)
{
	
    input_unregister_device(button_dev);
	free_irq(BUTTON_IRQ, NULL);
	gpio_free(S5PV210_GPH0(2));
}

module_init(button_init);
module_exit(button_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DREW 1677812132@qq.com");
MODULE_DESCRIPTION("MY_MODULES");