#ifndef WIREWAY_DEV_H
#define WIREWAY_DEV_H

#include <linux/init.h>
#include "wireway_node.h"
#define WIREWAY_IOCTL_MAGIC 0xF8


#define WIREWAY_IOC_CREATE_WIRE _IO(WIREWAY_IOCTL_MAGIC,1) 
#define WIREWAY_IOC_DELETE_WIRE _IO(WIREWAY_IOCTL_MAGIC,2)
#define WIREWAY_IOC_CREATE_COLLECTOR _IO(WIREWAY_IOCTL_MAGIC,3)
#define WIREWAY_IOC_DELETE_COLLECTOR _IO(WIREWAY_IOCTL_MAGIC,4)

#define wireway_msg_data 1
#define wireway_msg_reply 2
#define wireway_msg_data_with_reply 4



#define NIPQUAD(addr) \  
  ((unsigned char *)&addr)[0], \
  ((unsigned char *)&addr)[1], \
  ((unsigned char *)&addr)[2], \
  ((unsigned char *)&addr)[3]



struct udp_trans_wait_queue_head
{
    spinlock_t lock;
    struct list_head waitor_list;
};


struct wireway_dev_data
{
    unsigned int nr;
    char *dev_ptr;
};
struct wireway_msg_head
{
    char type;
    unsigned long key;
    unsigned long wireway_id;
};




int wireway_dev_init(void);
void wireway_dev_exit(void);
struct udp_trans_wait_queue_head* get_wait_queue_head(unsigned long wireway_id,char type);
int udp_trans_wake(struct udp_trans_wait_queue_head *head,    unsigned long key,void  *msg);
int packet_rcv_wakeup(void);
extern wireway_collector *collector_main;
#endif
