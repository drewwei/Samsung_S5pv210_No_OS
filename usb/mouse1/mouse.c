#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/usb.h>
#include <linux/hid.h>





static int mouse_probe(struct usb_interface *interface,  const struct usb_device_id *id)
{
		printk("This is mouse_probe.\n");
		return 0;
}


void mouse_disconnect(struct usb_interface *interface)
{
	printk("This is mouse_disconnect.\n");
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






































