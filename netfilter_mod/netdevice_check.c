#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/user_namespace.h>

#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include "wireway_dev.h"

void print_netdevice_info(void)
{
    struct net *net = NULL ;
    struct net_device *dev = NULL;
    char type[32];
    struct net_device_stats * stats = NULL;
    struct in_device *ind = NULL;
    struct in_ifaddr *ina = NULL; 
    
    rtnl_lock();
    for_each_net(net)
    {
        for_each_netdev(net,dev)
        {
            //判断netdevice的类型
            
            if (dev->type == ARPHRD_ETHER)
            {
                sprintf(type, "Ethernet");
            }
            else if (dev->type == ARPHRD_LOOPBACK)
            {
                sprintf(type, "Local Loopback");
            }
            else
            {
                sprintf(type, "%d", dev->type);
            }
            
            printk("%s\t", dev->name);
            printk("Link encap:%s  ", type);

            //打印MAC地址
            printk("HWaddr %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n",
            dev->dev_addr[0], dev->dev_addr[1],
            dev->dev_addr[2], dev->dev_addr[3],
            dev->dev_addr[4], dev->dev_addr[5]);

            //获取IP地址并打印
            ind = in_dev_get(dev);
            if (ind)
            {
                ina = (struct in_ifaddr *)ind->ifa_list;
                if (ina)
                {
                    printk("\tinet addr:%u.%u.%u.%u Bcast:%u.%u.%u.%u Mask:%u.%u.%u.%u\n",
                    NIPQUAD(ina->ifa_address),
                    NIPQUAD(ina->ifa_broadcast),
                    NIPQUAD(ina->ifa_mask));
                }
            }
 
            //状态信息
            if (dev->flags & IFF_UP) printk("UP ");
            if (dev->flags & IFF_BROADCAST) printk("BROADCAST ");
            if (dev->flags & IFF_RUNNING) printk("RUNNING ");
            if (dev->flags & IFF_LOOPBACK) printk("LOOPBACK ");
            if (dev->flags & IFF_MULTICAST) printk("MULTICAST ");
            //MTU信息
              printk("MTU:%d\n", dev->mtu);
              printk("\t");
              printk("\n");
        }

    }
    rtnl_unlock();
}
