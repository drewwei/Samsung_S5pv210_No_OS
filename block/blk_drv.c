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
		/*这个函数通过kmap_atomic()获得返回给定bio 的第i 个缓冲区的虚拟地址。*/
		ramblk_xfer(dev, sector, bio_cur_bytes(bio)>>9, buffer, bio_data_dir(bio));
		sector += bio_cur_bytes(bio)>>9;
		/*这个函数返还由_ _bio_kmap_atomic()获得的内核虚拟地址。*/
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
/*通过函数blk_alloc_queue分配一个默认的请求队列，
用该方法生成的请求对面没有设置默认的IO调度器。
如果调用blk_init_queue函数分配一个请求队列，会设置默认的IO调度器。
因为是编写ram disk，不需要访问外部设备，所以不需要使用IO调度器，
故使用blk_alloc_queue来分配一个请求队列。*/
	device->queue = blk_alloc_queue(GFP_KERNEL);
/* blk_alloc_queue分配的请求队列中make_request_fn是没有被赋值的，
这也导致了前面说的不会使用默认的IO调度器，
那么我们就必须自己实现这个函数，
因为上层代码向请求队列发生请求时都是通过这个函数来完成的。
因为我们使用内存来模拟块设备，所以其实连请求队列都不需要，
上面分配它仅仅为了让上层代码能够使用请求队列中的make_request_fn函数，
否则上层代码会不知道去哪里调用make_request_fn。
*/
	blk_queue_make_request(device->queue, ramblk_make_request);
	
	//device->queue->make_request_fn = ramblk_make_request;
	device->queue->queuedata = device;
/*分配gendisk*/
	device->gd = alloc_disk(16);   
/*设置gendisk*/
	device->gd->queue = device->queue;
	device->gd->major = major;
	device->gd->first_minor = 0;
	//device->gd->disk_name = "ramdisk";
	sprintf(device->gd->disk_name, "ramblk");
	device->gd->fops = &ramblk_fops;
	
	//device->gd->private_data = device
/*设置gendisk容量,磁盘一共有多少个扇区*/
	set_capacity(device->gd, size);
/*注册gendisk*/
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




