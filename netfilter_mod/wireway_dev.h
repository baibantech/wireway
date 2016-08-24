#include <linux/init.h>

#define WIREWAY_IOCTL_MAGIC 0xF8


#define WIREWAY_IOC_CREATE_WIRE _IO(WIREWAY_IOCTL_MAGIC,1) 
#define WIREWAY_IOC_DELETE_WIRE _IO(WIREWAY_IOCTL_MAGIC,2)
#define WIREWAY_IOC_CREATE_COLLECTOR _IO(WIREWAY_IOCTL_MAGIC,3)
#define WIREWAY_IOC_DELETE_COLLECTOR _IO(WIREWAY_IOCTL_MAGIC,4)



#define NIPQUAD(addr) \  
  ((unsigned char *)&addr)[0], \
  ((unsigned char *)&addr)[1], \
  ((unsigned char *)&addr)[2], \
  ((unsigned char *)&addr)[3]



struct wireway_dev_data
{
    unsigned int nr;
    char *dev_ptr;
};
int wireway_dev_init(void);
void wireway_dev_exit(void);
