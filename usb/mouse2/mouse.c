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
	/*����֧�ֵ��¼�����*/
	set_bit(EV_KEY, dev->evbit);
	//t_bit(EV_REP, dev->evbit);
	/*����֧�ֵľ����¼����͵���Щ�¼�*/
	set_bit(KEY_L, dev->keybit);
	set_bit(KEY_S, dev->keybit);
	set_bit(KEY_ENTER, dev->keybit);
	/*ע��input_dev*/
	ret = input_register_device(dev);
	if(ret != 0) {
		printk("input_register_device fail.\n");
		input_free_device(dev);
		return -EINVAL;
		}


/*�жϽ���Ķ˵�����ǲ��ǵ���1��USB���涨�˵����Ϊ1*/
	hintf = interface->cur_altsetting;
	desc = interface->cur_altsetting->desc;
	if(desc.bNumEndpoints != 1) {	
			printk("This is not a mouse!\n");
			return -EINVAL;
		}
	ep = &hintf->endpoint[0].desc;
	printk("%d.\n",ep->bmAttributes);
/*����bit7 ��ʾ *��������˵�ķ���0 ��ʾOUT��1 ��ʾIN��OUT ��IN �Ƕ��������ԡ�*/	
	if(!(ep->bEndpointAddress & 0x80)) {
		printk("Is not in\n");
		return -ENODEV;
		}
/*00 ��ʾ����,01 ��ʾ��ʱ,10 ��ʾ����,11 ��ʾ�ж�*/	
	if((ep->bmAttributes & 0x3) != 3) {
		printk("Is not interrupt\n");
		return -ENODEV;
		}
/*����һ���ܵ�pipe    �����������ж�*/

	pipe = usb_rcvintpipe(udev, ep->bEndpointAddress);
	//maxp = usb_maxpacket(udev, pipe, 0);
	maxp = udev->ep_in[0]->desc.wMaxPacketSize;


/* 
         ��������urb�������ݴ�����ڴ棬ע�⣺���ｫ���ء"buff���͡�mdama��  
         buff����¼��������ͨ�����õ��ڴ�ָ��  
         mdma����¼������DMA������ڴ�ָ��  
         �����DMA ��ʽ�Ĵ��䣬��ôusb core ��Ӧ��ʹ��mdma
     */  
	buff = usb_alloc_coherent(udev, maxp, GFP_ATOMIC, &mdma);
	if(NULL == buff) {
		printk("usb_alloc_coherent fali\n");
		return -ENOMEM;
	}

/*���䲢����urb*/
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
/*�ϱ�urb*/
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






































