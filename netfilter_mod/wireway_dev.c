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
#include <linux/delay.h>
#include <linux/cpumask.h>
#include "wireway_dev.h"
#include "wireway_node.h"

dev_t wireway_dev;
int dev_major = 0,dev_minor = 0;
struct cdev *wireway_cdev = NULL;
extern int msg_rcv;
struct class *wireway_class = NULL;
extern void print_netdevice_info(void);
extern void kernel_udp_sock_test(struct net *net);
extern void kernel_udp_sock_realse(void);
extern int send_udp_packet_test(void);

wireway_collector *collector_main = NULL;
struct timer_list wireway_timer = {0};
struct timer_list wireway_debug_timer = {0};


extern unsigned long msg_in_queue_av;
extern unsigned long msg_in_queue_max ;
extern unsigned long wake_av ;
extern unsigned long wake_max ;


enum kthread_state
{
    thread_idle,
    thread_wait  ,
    thread_running,
    thread_run,
};

typedef struct packet_process_control
{
    int thread_num;
    struct task_struct*  task[NR_CPUS];
    volatile atomic_t  task_state[NR_CPUS];
    atomic_t  task_wake;
    volatile unsigned long task_wake_num[NR_CPUS];
    volatile unsigned long task_wake_num1[NR_CPUS];
    volatile unsigned long task_wait_num[NR_CPUS];
    volatile unsigned long task_wait_num1[NR_CPUS];
    volatile unsigned long task_leave_wait[NR_CPUS];
}packet_process_control;

packet_process_control *p_control = NULL;

void show_packet_rcv_control(void)
{
    int i = 0;
    for(i = 0 ;i <p_control->thread_num ; i++)
    {   
        printk("-------------------------------------------\r\n");
        printk("cpu  %d,task state %d\r\n",i,p_control->task[i]->state);
        printk("cpu  %d,task_state %d\r\n",i,p_control->task_state[i].counter);
        printk("cpu  %d,task wait num %lld\r\n",i,p_control->task_wait_num[i]);
        printk("cpu  %d,task wait1 num %lld\r\n",i,p_control->task_wait_num1[i]);
        printk("cpu  %d,task leave num %lld\r\n",i,p_control->task_leave_wait[i]);
        printk("cpu  %d,task wakeup num %lld\r\n",i,p_control->task_wake_num[i]);
        printk("cpu  %d,task wakeup1 num %lld\r\n",i,p_control->task_wake_num1[i]); 
    }
}

void packet_rcv_wait(int cpu)
{
    int old_state;
    old_state = atomic_xchg(&p_control->task_state[cpu],thread_wait);
    if(thread_run == old_state)
    {
        atomic_xchg(&p_control->task_state[cpu],thread_running);
        p_control->task_wait_num[cpu]++;
        barrier();
        schedule();
    }
    else
    {
        set_current_state(TASK_UNINTERRUPTIBLE);
        p_control->task_wait_num1[cpu]++;
        barrier();
        schedule();
        atomic_xchg(&p_control->task_state[cpu],thread_running);
    }
    p_control->task_leave_wait[cpu]++;
}


int packet_rcv_wakeup(void)
{
    int old_state;
    int thread_num = p_control->thread_num;
    int i;
    int wake_thread = 0;
    for(i = 0; i < thread_num ; i++)
    {
        if(smp_processor_id () == i)
        {
            continue;
        }
        old_state = atomic_xchg(&p_control->task_state[i],thread_run);
        if((old_state != thread_run) && (old_state != thread_running))
        {
            //printk("wake up process %d\r\n",i);
            p_control->task_wake_num[i]++;
            while(0 == wake_up_process(p_control->task[i]))
            {
                barrier();
                if(p_control->task_state[i].counter != thread_running)
                {
                    break;
                }
            }
            return 1;            
        }
               
    }
    #if 0
    while(1)
    {
        wake_thread = atomic_add_return(1,&p_control->task_wake);
        wake_thread = wake_thread%p_control->thread_num;
        if(wake_thread != smp_processor_id())
        {
     
            old_state = atomic_xchg(&p_control->task_state[wake_thread],thread_run);
            if(thread_wait == old_state)
            {
              //  printk("wake up process %d\r\n",wake_thread);
                p_control->task_wake_num1[i]++;
                while(0 == wake_up_process(p_control->task[i]))
                {
                    if(p_control->task_state[i].counter != thread_running)
                    {
                        break;
                    }
                }

            }
            break;
        }
    }    
    #endif
    return 0;
}

static int packet_rcv_process(void *args)
{
    void *msg = NULL;
    int cpu = smp_processor_id();
    unsigned long long idx = 0;
    int msg_count = 0;
    int count = 0;
    unsigned int local_pmt = 0;
    unsigned int read_cnt = 0;
    unsigned int local_idx = 0;
    collector_main  = lookup_collector("collector/main");
    if(NULL == collector_main)
    {
        printk("collector error\r\n");
        return -1;
    }
    lfrwq_t *qh = collector_main->rcv_queue;
    printk("thread %d run \r\n",cpu)  ;  
    do
    {
        local_pmt = lfrwq_get_rpermit(qh);
        
        if(0 == local_pmt)
        {
            count++;
            if(100 == count)
            {
                packet_rcv_wait(cpu);
                count = 0;
            }
        }
        else
        {
            //printk("pmt is %d\r\n",local_pmt);
        }

        while(local_pmt > 0)
        { 
            idx = lfrwq_deq(qh,&msg);
            if(msg)
            {
                if(msg == 0xABABABABBABABABA)
                {
                    //printk("msg soft in q\r\n");
                
                }
                else
                {
                    kfree(msg);
                }
                msg_count++;

                if(msg_count > 100)
                {
                    msg_count = 0;
                    schedule();
                }
            }    
            local_pmt--;
            
            if(0 == read_cnt)
            {
                local_idx = idx;
                read_cnt++;
                continue;
            }

            if(idx != local_idx)
            {
                //printk("local idx is %d,idx is %d,read_cnt is %lld\r\n",local_idx,idx,read_cnt);
                lfrwq_add_rcnt(qh,read_cnt,local_idx);
                read_cnt = 0;
                local_idx = idx;
            }
            
            read_cnt++;       
        }

        if(read_cnt)
        {
            lfrwq_add_rcnt(qh,read_cnt,local_idx);
            read_cnt = 0;
        }
    
        
    }while(!kthread_should_stop());
    return 0;        
}
static void wireway_packet_timer(unsigned long data)
{
    static int local_w_idx = 1;
    static int local_count = 0;

   // show_packet_rcv_control();
    if(collector_main)
    {
        lfrwq_t *qh = collector_main->rcv_queue;
        if(local_w_idx == qh->w_idx)
        {
            if(local_count++ >5)
            {
                local_count = 0;
                //printk("msg_rcv is %d\r\n",msg_rcv);
                lfrwq_soft_inq(qh,local_w_idx);                
            }   
        }
        else
        {
            local_w_idx = qh->w_idx;
            local_count = 0;
        } 
    }
    
    wireway_timer.data = 0;
    wireway_timer.expires = jiffies + 50;
    add_timer(&wireway_timer);
   
}

static void debug_timer_process(unsigned long data)
{
    show_packet_rcv_control();
    if(collector_main)
    {
        lfrwq_t *qh = collector_main->rcv_queue;
        printk("queue rd idx is %lld\r\n",qh->r_idx);
        printk("queue wd idx is %lld\r\n",qh->w_idx);
    }
    show_long_val(msg_in_queue_av);
    show_long_val(msg_in_queue_max);
    show_long_val(wake_av);
    show_long_val(wake_max);
    wireway_debug_timer.data = 0;
    wireway_debug_timer.expires = jiffies + 10*HZ;
    add_timer(&wireway_debug_timer); 
}

int packet_process_init(void)
{
    int cpu = 0;
    struct timer_list my_timer = {0};
    p_control = kmalloc(sizeof(packet_process_control),GFP_KERNEL);
    if(p_control)
    {
        memset(p_control,0,sizeof(packet_process_control));
        printk("current cpu is %d\r\n",smp_processor_id());       
        for_each_online_cpu(cpu)
        {
            atomic_set(&p_control->task_state[cpu] ,thread_idle);
            p_control->task[cpu] = kthread_create_on_node(packet_rcv_process,NULL,cpu_to_node(cpu),"pkt_rcv_%d",cpu);
            p_control->thread_num++;
            if(!IS_ERR(p_control->task[cpu]))
            { 
                kthread_bind(p_control->task[cpu],cpu);
                wake_up_process(p_control->task[cpu]);
            }
            else
            {
                printk("init kernel thread err\r\n");
            }

        }  
        
    }
    
    init_timer(&wireway_timer);
    wireway_timer.function = wireway_packet_timer;
    wireway_timer.data = 0;
    wireway_timer.expires = jiffies + 50;
    add_timer(&wireway_timer);
    
    init_timer(&wireway_debug_timer);
    wireway_debug_timer.function = debug_timer_process;
    wireway_debug_timer.data = 0;
    wireway_debug_timer.expires = jiffies + 10*HZ;
    add_timer(&wireway_debug_timer);
    printk("HZ is %d\r\n",HZ);         
    show_packet_rcv_control();
    return 0;
}

void packet_process_release(void)
{
    int cpu ;
    if(p_control)
    {
        for_each_online_cpu(cpu)
        {
            if(p_control->task[cpu])
            {
                kthread_stop(p_control->task[cpu]);
            }
        }
        kfree(p_control);
    }
    del_timer(&wireway_timer);
    del_timer(&wireway_debug_timer);
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
    unsigned long  collector_id = 0;
    wireway_collector *collector = NULL;
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

    collector_id = create_collector("collector/main");
    printk("main collector id is %d\r\n",collector_id);
    collector = cache_id_lookup(collector_cache,collector_id);
    printk("collector addr is 0x%p\r\n",collector);
    collector = lookup_collector("collector/main");
    printk("collector addr is 0x%p\r\n",collector);
    
    packet_process_init(); 

    /*need reg self user entity*/
    //print_netdevice_info();     
    //kernel_udp_sock_test(NULL);
    //kernel_udp_sock_realse();
    //send_udp_packet_test();
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
    
    packet_process_release();

    return;

}
