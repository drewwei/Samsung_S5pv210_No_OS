#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <linux/errno.h>
#include <linux/timer.h>


#define  GPH0_1	 S5PV210_GPH0(1)
#define  GPH0_2	 S5PV210_GPH0(2)
#define  GPH0_3	 S5PV210_GPH0(3)

#define  IRQ_L			IRQ_EINT1
#define  IRQ_S      		IRQ_EINT2		//GPH0_2
#define 	IRQ_ENTER		IRQ_EINT3		//GPH0_3
static void fcn(unsigned long data);

static struct input_dev * dev;

static DEFINE_TIMER(my_timer, fcn, 0, 0 );  /*¶¨Òå²¢³õÊ¼»¯Ò»¸öÄÚºË¶¨Ê±Æ÷*/
struct button_info{
	unsigned int irq_num;
	char *name;
	unsigned int gpio;
	unsigned int code;
	};
struct  button_info *id;

static struct button_info irqs[] = {
		{
			.irq_num = IRQ_L,
			.name     = "button1",
			.gpio	= GPH0_1,
			.code	= KEY_L,
		},
		{
			.irq_num = IRQ_S,
			.name     = "button2",
			.gpio	= GPH0_2,
			.code= KEY_S,
		}, 
		{
			.irq_num = IRQ_ENTER,
			.name     = "button3",
			.gpio	= GPH0_3,
			.code= KEY_ENTER,
		}, 
	};


/*static struct timer_list my_timer;  ¶¨ÒåÒ»¸öÄÚºË¶¨Ê±Æ÷

*init_timer(&my_timer);   ³õÊ¼»¯timer_listµÄentryµÄnextÎªNULL£¬²¢¸øbaseÖ¸Õë¸³Öµ
*TIMER_INITIALIZER(_function, _expires, _data);  data£©ºêÓÃÓÚ¸³Öµ¶¨Ê±Æ÷½á¹¹ÌåµÄfunction¡¢expires¡¢
*dataºÍbase³ÉÔ±£¬
*DEFINE_TIMER(_name, _function, _expires, _data); ºêÊÇ¶¨Òå²¢³õÊ¼»¯¶¨Ê±Æ÷³ÉÔ±µÄ¡°¿ì½Ý·½Ê½¡
*add_timer(struct timer_list * timer) //ÏòÄÚºË×¢²á¶¨Ê±Æ÷
*del_timer(struct timer_list * timer)  //É¾³ý¶¨Ê±Æ÷
*mod_timer(struct timer_list * timer, unsigned long expires)  //ÐÞ¸Ä¶¨Ê±Æ÷*/


 
static irqreturn_t irq(int irq, void *dev_id) 
{
	id=(struct button_info *)dev_id;
	
	mod_timer(&my_timer, jiffies+HZ/100);
	
	return IRQ_HANDLED;
}

 static void fcn(unsigned long data)
 {	
	unsigned int value = 0;
	value = __gpio_get_value(id->gpio);
	
	//printk("This is %s irq.value is :%d.\n",id->name,value);
	
	input_report_key(dev, id->code, !value);
	input_sync(dev);	
 }


static int __init button_init(void)
{
	int error, i;
	
	dev = input_allocate_device();
	if(!dev){
		printk("input_allocate_device fail.\n");
		return -ENOMEM;
		}
	set_bit(EV_KEY, dev->evbit);
	set_bit(KEY_L, dev->keybit);
	set_bit(KEY_S, dev->keybit);
	set_bit(KEY_ENTER, dev->keybit);
	set_bit(REP_DELAY, dev->keybit);
	dev->name = "button";

	error = input_register_device(dev);
	if(error){
		printk("input_register_device fail.\n");
		goto err_free_dev;
		}
	for(i = 0; i<3; i++){
		if(request_irq(irqs[i].irq_num,  irq,  IRQ_TYPE_EDGE_FALLING, irqs[i].name, &irqs[i])) {
			printk("request_irq fail.\n");
			goto err_request_irq;
			}
		}
	add_timer(&my_timer);
	return 0;

err_request_irq:
	input_unregister_device(dev);
err_free_dev:
	input_free_device(dev);
	return error;
}

static void __exit button_exit(void)
{
	int i;
	input_unregister_device(dev);

	for(i = 0; i<3; i++){
		free_irq(irqs[i].irq_num, &irqs[i]);
	}
	del_timer(&my_timer);  /* ÏòÄÚºË×¢²á¶¨Ê±Æ÷*/
}


module_init(button_init);
module_exit(button_exit);
MODULE_LICENSE("GPL");





























