#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include "wireway_dev.h"
#include "wireway_node.h"

dev_t wireway_dev;
int dev_major = 0,dev_minor = 0;
struct cdev *wireway_cdev = NULL;
extern void print_netdevice_info(void);
extern void kernel_udp_sock_test(struct net *net);
extern void kernel_udp_sock_realse(void);
extern int send_udp_packet_test(void);


int select_spider_addr(char *local_name,int local_id)
{
    return 1;
}



int wireway_dev_open(struct inode *inode,struct file *filp)
{
    return 0;
}
static size_t wireway_dev_read(struct file *filp,char __user *buf,size_t size,loff_t *ppos)
{
    return 0;
}

static size_t wireway_dev_write(struct file *filp,const char __user *buf,size_t size,loff_t *ppos)
{
    return 0;
}

int wireway_dev_ioctl(struct file *filp,unsigned int cmd,unsigned long args)
{
    switch(cmd)
    {
        #if 0
        case WIREWAY_IOC_CREATE_WIRE:
            {
                char *wireway_name = (void*)args;

                
                int spider_addr  = select_spider_addr();
                
                ret = call_for_create_wireway(spider_addr,wireway_name);
                
                                


   
            } 
                
             
            
            break;
        #endif
        case WIREWAY_IOC_CREATE_COLLECTOR:
            {
                char *collector_name = (void*)args;
                if(!lookup_collector(collector_name))
                {
                    create_collector(collector_name);
                }
                else
                {
                    return -1;
                }

            }       


            break;
        default : printk("error cmd number\r\n");break;
    }
    return 0;

}


int wireway_dev_mmap(struct file *filp,struct vm_area_struct *vm)
{
    unsigned long addr = vm->vm_start;
    unsigned long collector_id =vm->vm_pgoff;
    wireway_collector *collector = cache_id_lookup(collector_cache,collector_id);
    if(collector)
    {
        unsigned long que_addr = collector->rcv_queue;
        unsigned long que_phy_addr = virt_to_phys(que_addr);
        unsigned long size = vm->vm_end - vm->vm_start; 
        if(remap_pfn_range(vm,addr,que_phy_addr>>PAGE_SHIFT,size,PAGE_SHARED))
        return -1;  

    }

    return 0;
}

#if 0
unsigned long  wireway_dev_get_umaped_area(struct file *filp, unsigned long addr , unsigned long len, unsigned long prot, unsigned long offset)
{


    #if 0

    unsigned long (*get_area)(struct file *, unsigned long,
                  unsigned long, unsigned long, unsigned long);


    unsigned long collector_id = addr;
    wireway_collector *collector = cache_id_lookup(collector_cache,collector_id);
    

    if(wireway_collector)
    {
        get_area =  current->mm->get_unmapped_area;
         
         


   
        
    }
    #endif


    

}
#endif

static const struct file_operations wireway_dev_fops = 
{
    .owner = THIS_MODULE,
    .read =  wireway_dev_read,
    .write = wireway_dev_write,
    .open = wireway_dev_open,
    .unlocked_ioctl = wireway_dev_ioctl,
    .mmap = wireway_dev_mmap,
    #if 0
    .get_unmaped_area = wireway_dev_get_unmap_area, 
    #endif
};

int wireway_dev_init(void)
{
    int result ;
    dev_t devno = MKDEV(dev_major,0);
    printk("hello wireway dev init\r\n");
    if(dev_major)
    {
        result = register_chrdev_region(devno,1,"wireway_dev");
    }
    else
    {
        result = alloc_chrdev_region(&devno,0,1,"wireway_dev");
        dev_major = MAJOR(devno);
    }
    if(result < 0)
    {
        return result;
    }

    wireway_cdev = cdev_alloc();
    if(!wireway_cdev)
    {
        return -1;
    }

    wireway_cdev->owner = THIS_MODULE;
    wireway_cdev->ops = &wireway_dev_fops;
    
    #if 1
    result = cdev_add(wireway_cdev,MKDEV(dev_major,0),1);

    if(result < 0)
    {
        printk("cdev_add return err\r\n");
        return result;
    }
    #endif

    /*need reg self user entity*/
    //print_netdevice_info();     
    //kernel_udp_sock_test(NULL);
    //kernel_udp_sock_realse();
    send_udp_packet_test();
    return 0;
}
void wireway_dev_exit(void)
{
    unregister_chrdev_region(MKDEV(dev_major,0),1);
    if(wireway_cdev)
    {
        cdev_del(wireway_cdev);        
    }

    return;

}
