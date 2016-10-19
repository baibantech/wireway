#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/threads.h>
#include <linux/kthread.h>
#include <linux/cpumask.h>
#include "wireway_dev.h"
#include "wireway_node.h"

dev_t wireway_dev;
int dev_major = 0,dev_minor = 0;
struct cdev *wireway_cdev = NULL;

struct class *wireway_class = NULL;
extern void print_netdevice_info(void);
extern void kernel_udp_sock_test(struct net *net);
extern void kernel_udp_sock_realse(void);
extern int send_udp_packet_test(void);

enum kthread_state
{
    thread_wait = 1 ,
    thread_running,
    thread_run,
};

typedef struct packet_process_control
{
    int thread_num;
    struct task_struct*  task[NR_CPUS];
    atomic_t  task_state[NR_CPUS];
    atomic_t  task_wake;
}packet_process_control;

packet_process_control *p_control = NULL;

void packet_rcv_wait(int cpu)
{
    int old_state;
    set_current_state(TASK_UNINTERRUPTIBLE);
    old_state = atomic_xchg(&p_control->task_state[cpu],thread_wait);
    if(thread_run == old_state)
    {
        set_current_state(TASK_RUNNING);
        schedule();
    }
    else
    {
        schedule();
    }
    atomic_xchg(&p_control->task_state[cpu],thread_running);
}


int packet_rcv_wakeup(void)
{
    int old_state;
    int thread_num = p_control->thread_num;
    int i;
    int wake_thread = 0;
    for(i = 0; i < thread_num ; i++)
    {
        if(thread_wait == atomic_cmpxchg(&p_control->task_state[i],thread_wait,thread_run))
        {
            while(!wake_up_process(p_control->task[i]));
            return 1;            
        }
               

    }
    wake_thread = atomic_add_return(1,&p_control->task_wake);
    wake_thread = wake_thread%p_control->thread_num; 
    old_state = atomic_xchg(&p_control->task_state[wake_thread],thread_run);
    if(thread_wait == old_state)
    {
        while(!wake_up_process(p_control->task[wake_thread]));
    } 
    return 0;
}

static int packet_rcv_process(void *args)
{
    wireway_collector *collector = lookup_collector("collector/main");
    lfrwq_t *qh = collector->rcv_queue;
    void *msg = NULL;
    int cpu = *(int*)args;
    unsigned long long idx = 0;
    int count;
    int msg_count = 0;
    atomic_xchg(&p_control->task_state[cpu],thread_running);
    do
    {
        idx = lfrwq_deq(qh,&msg);
        if(NULL != msg)
        {
            count = 0;
            //set_msg_index(idx,msg);
            //handle_msg();
            printk("rcv msg\r\n");
            msg_count++;
            continue;
        }
        else
        {
            count++;
        }
        
        if(10 == msg_count)
        {
            
        }

        if(count++ == 10)
        {
            packet_rcv_wait(cpu);
        }        
        
    }while(kthread_should_stop());
    return 0;        
}


int packet_process_init(void)
{
    int cpu = 0;
    p_control = kmalloc(sizeof(packet_process_control),GFP_KERNEL);
    if(p_control)
    {
        memset(p_control,0,sizeof(packet_process_control));
       
        for_each_online_cpu(cpu)
        {
            p_control->task[cpu] = kthread_create(packet_rcv_process,&cpu,"pkt_rcv_%d",cpu);
            atomic_set(&p_control->task_state[cpu] ,thread_wait);
            p_control->thread_num++;
            kthread_bind(p_control->task[cpu],cpu);
            wake_up_process(p_control->task[cpu]); 
        }  
        
    }
    create_collector("collector/main");


}

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
         
        case WIREWAY_IOC_GET_COLLECTOR_QUE_LEN:

        {
            char *collector_name = (void*)args;
             


            break;
        }
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
    wireway_class = class_create(THIS_MODULE,"wireway_class");
    
    if(IS_ERR(wireway_class))
    {
        printk("create class error\r\n");
        return -1;
    }
    
    device_create(wireway_class,NULL,devno,NULL,"wireway");
 
    wireway_dev = devno;
    
    collector_cache_init();


    packet_process_init(); 

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
    device_destroy(wireway_class,wireway_dev);
    class_destroy(wireway_class);    

    return;

}
