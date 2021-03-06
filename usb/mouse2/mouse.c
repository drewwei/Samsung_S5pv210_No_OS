#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/usb.h>
#include <linux/hid.h>
#include <linux/device.h>
#include <linux/input.h>
static struct input_dev *dev;
static void usb_mouse_irq(struct urb *urb)
{
/*
	int i;
	char *buff = urb->transfer_buffer;
	for(i = 0; i < urb->transfer_buffer_length; i++) {
		printk("%02x",buff[i]);	
		}
	printk("\n");
	*/

	char *buff = urb->transfer_buffer;
	static char pre_val ;
	if((pre_val & (1<<0)) != (buff[1] & (1<<0)))  {
		input_report_key(dev, KEY_L, (buff[1] & (1<<0)) ? 1 : 0);
		input_sync(dev);
		}
	if((pre_val & (1<<1)) != (buff[1] & (1<<1))) {
		input_report_key(dev, KEY_S, (buff[1] & (1<<1)) ? 1 : 0);
		input_sync(dev);
		}
	if((pre_val & (1<<2)) != (buff[1]& (1<<2))) {
		input_report_key(dev, KEY_ENTER, (buff[1] & (1<<2 )) ? 1 : 0);
		input_sync(dev);
		}
	pre_val = buff[1];

	usb_submit_urb(urb, GFP_KERNEL);
}
static int mouse_probe(struct usb_interface *interface,  const struct usb_device_id *id)
{
	//printk("This is mouse_probe.\n");
	struct usb_device * udev = interface_to_usbdev(interface);
	struct usb_host_interface *hintf;
	struct usb_interface_descriptor  desc;
	struct usb_endpoint_descriptor *ep;
	int ret;
	int pipe;
	unsigned short maxp;
	char * buff;
	dma_addr_t mdma;
	struct urb *urb;

	
	dev = input_allocate_device();
	if(dev == NULL) {
		printk("input_allocate_device fail\n");
		return -EINVAL;
		}
	dev->name = "usb_mouse";
	/*设置支持的事件类型*/
	set_bit(EV_KEY, dev->evbit);
	//t_bit(EV_REP, dev->evbit);
	/*设置支持的具体事件类型的哪些事件*/
	set_bit(KEY_L, dev->keybit);
	set_bit(KEY_S, dev->keybit);
	set_bit(KEY_ENTER, dev->keybit);
	/*注册input_dev*/
	ret = input_register_device(dev);
	if(ret != 0) {
		printk("input_register_device fail.\n");
		input_free_device(dev);
		return -EINVAL;
		}


/*判断接入的端点个数是不是等于1，USB鼠标规定端点个数为1*/
	hintf = interface->cur_altsetting;
	desc = interface->cur_altsetting->desc;
	if(desc.bNumEndpoints != 1) {	
			printk("This is not a mouse!\n");
			return -EINVAL;
		}
	ep = &hintf->endpoint[0].desc;
	printk("%d.\n",ep->bmAttributes);
/*其中bit7 表示 *的是这个端点的方向，0 表示OUT，1 表示IN，OUT 与IN 是对主机而言。*/	
	if(!(ep->bEndpointAddress & 0x80)) {
		printk("Is not in\n");
		return -ENODEV;
		}
/*00 表示控制,01 表示等时,10 表示批量,11 表示中断*/	
	if((ep->bmAttributes & 0x3) != 3) {
		printk("Is not interrupt\n");
		return -ENODEV;
		}
/*构建一个管道pipe    类型是输入中断*/

	pipe = usb_rcvintpipe(udev, ep->bEndpointAddress);
	//maxp = usb_maxpacket(udev, pipe, 0);
	maxp = udev->ep_in[0]->desc.wMaxPacketSize;


/* 
         申请用于urb用于数据传输的内存，注意：这里将返回�"buff”和“mdama”  
         buff：记录了用于普通传输用的内存指针  
         mdma：记录了用于DMA传输的内存指针  
         如果是DMA 方式的传输，那么usb core 就应该使用mdma
     */  
	buff = usb_alloc_coherent(udev, maxp, GFP_ATOMIC, &mdma);
	if(NULL == buff) {
		printk("usb_alloc_coherent fali\n");
		return -ENOMEM;
	}

/*分配并设置urb*/
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if(NULL == urb) {
		printk("usb_alloc_urb fail\n");
		usb_free_coherent(udev, maxp, buff, mdma);
		return -ENOMEM;		
		}
	printk("I am debug1!\n");
	usb_fill_int_urb(urb, udev, pipe, buff, maxp, usb_mouse_irq, NULL, ep->bInterval);
	
	urb->transfer_flags |=URB_NO_TRANSFER_DMA_MAP;
	urb->transfer_dma= mdma;

	dev_set_drvdata(&interface->dev, urb);
/*上报urb*/
	usb_submit_urb(urb, GFP_KERNEL);

	printk("I am debug2!\n");

	return 0;
}


void mouse_disconnect(struct usb_interface *interface)
{
	struct urb *urb = dev_get_drvdata(&interface->dev);
	
	printk("This is mouse_disconnect.\n");
	input_unregister_device(dev);
	input_free_device(dev);
	usb_kill_urb(urb);
	usb_free_urb(urb);
}

static const struct usb_device_id mouse_id[] = {	
	{ USB_DEVICE(0x1bcf, 0x0007) },

	//{USB_DEVICE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_MOUSE)},
	{ }						
};

static struct usb_driver driver = {
		.name = "my_mouse",
		.probe  = mouse_probe,
		.disconnect = mouse_disconnect,
		.id_table = mouse_id,
};

static int __init mouse_init(void)
{
	
	usb_register(&driver);
	return 0;
}

static void __exit mouse_exit(void)
{

	usb_deregister(&driver);
}

module_init(mouse_init);
module_exit(mouse_exit);
MODULE_LICENSE("GPL");






































