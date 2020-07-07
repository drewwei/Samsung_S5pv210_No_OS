#include <linux/module.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>

#define  size		1024
#define  KERNEL_SECTOR_SIZE  512
static  int major = 0;

struct blk_dev{
	unsigned char * 			buffer;
	struct request_queue *		queue;
	struct gendisk *			gd;
	unsigned long 				disksize;
};
static struct blk_dev *device;
static void ramblk_xfer(struct blk_dev *dev, unsigned long sector, unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector * KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect * KERNEL_SECTOR_SIZE;
	if((offset+nbytes) > dev->disksize) {
		printk("Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	if(write) {
		memcpy(device->buffer+offset, buffer, nbytes);
	}
	else{
		memcpy(buffer, device->buffer+offset, nbytes);
	}
	return;
}


static int ramblk_xfer_bio(struct blk_dev*dev, struct bio *bio)
{
	int i;
	struct bio_vec *bvec;
	sector_t sector = bio->bi_sector;
	bio_for_each_segment(bvec, bio, i) {
		char *buffer = __bio_kmap_atomic(bio, i, KM_USER0);
		/*�������ͨ��kmap_atomic()��÷��ظ���bio �ĵ�i ���������������ַ��*/
		ramblk_xfer(dev, sector, bio_cur_bytes(bio)>>9, buffer, bio_data_dir(bio));
		sector += bio_cur_bytes(bio)>>9;
		/*�������������_ _bio_kmap_atomic()��õ��ں������ַ��*/
		__bio_kunmap_atomic(bio, KM_USER0);
	}
	
	return 0;
}

static int ramblk_make_request(struct request_queue *queue, struct bio *bio)
{
	struct blk_dev *dev = queue->queuedata;
	int status;
	status = ramblk_xfer_bio(dev, bio);
	//bio_endio(bio, bio->bi_size, status);
	bio_endio(bio, status);
	return 0;
}

static const struct block_device_operations ramblk_fops = {
	.owner		= THIS_MODULE,
};

static int __init ramblk_init(void)
{
	major = register_blkdev(0, "ramblk");
	if(major <= 0) {
		printk(KERN_WARNING "unable to get major number\n");
		return -EBUSY;
	}
	device = kzalloc(sizeof(struct blk_dev), GFP_KERNEL);
	if(NULL == device) {
		printk("blk_dev kmalloc fail\n");
		return -ENOMEM;
	}
	device->disksize = size*KERNEL_SECTOR_SIZE;
/*ͨ������blk_alloc_queue����һ��Ĭ�ϵ�������У�
�ø÷������ɵ��������û������Ĭ�ϵ�IO��������
�������blk_init_queue��������һ��������У�������Ĭ�ϵ�IO��������
��Ϊ�Ǳ�дram disk������Ҫ�����ⲿ�豸�����Բ���Ҫʹ��IO��������
��ʹ��blk_alloc_queue������һ��������С�*/
	device->queue = blk_alloc_queue(GFP_KERNEL);
/* blk_alloc_queue��������������make_request_fn��û�б���ֵ�ģ�
��Ҳ������ǰ��˵�Ĳ���ʹ��Ĭ�ϵ�IO��������
��ô���Ǿͱ����Լ�ʵ�����������
��Ϊ�ϲ������������з�������ʱ����ͨ�������������ɵġ�
��Ϊ����ʹ���ڴ���ģ����豸��������ʵ��������ж�����Ҫ��
�������������Ϊ�����ϲ�����ܹ�ʹ����������е�make_request_fn������
�����ϲ����᲻֪��ȥ�������make_request_fn��
*/
	blk_queue_make_request(device->queue, ramblk_make_request);
	
	//device->queue->make_request_fn = ramblk_make_request;
	device->queue->queuedata = device;
/*����gendisk*/
	device->gd = alloc_disk(16);   
/*����gendisk*/
	device->gd->queue = device->queue;
	device->gd->major = major;
	device->gd->first_minor = 0;
	//device->gd->disk_name = "ramdisk";
	sprintf(device->gd->disk_name, "ramblk");
	device->gd->fops = &ramblk_fops;
	
	//device->gd->private_data = device
/*����gendisk����,����һ���ж��ٸ�����*/
	set_capacity(device->gd, size);
/*ע��gendisk*/
	add_disk(device->gd);

	device->buffer =(unsigned char *) kzalloc(size*512, GFP_KERNEL);
	return 0;
}

static void __exit ramblk_exit(void)
{	

	unregister_blkdev(major, "ramblk");
	del_gendisk(device->gd);
	put_disk(device->gd);
	blk_cleanup_queue(device->queue);
	kfree(device);
	
}




module_init(ramblk_init);
module_exit(ramblk_exit);

MODULE_LICENSE("GPL");




