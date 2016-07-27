#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <linux/types.h>  
#include <linux/in.h>  
#include <linux/ip.h> 
#include <linux/udp.h> 
#include <linux/netdevice.h>  
#include <linux/skbuff.h>  
#include <linux/netfilter_ipv4.h>  
#include <linux/inet.h>  
#include <linux/inetdevice.h>  
MODULE_LICENSE("GPL");  
#define NIPQUAD(addr) \  
  ((unsigned char *)&addr)[0], \  
  ((unsigned char *)&addr)[1], \  
  ((unsigned char *)&addr)[2], \  
  ((unsigned char *)&addr)[3]  
  
static unsigned int sample(  
unsigned int hooknum,  
struct sk_buff * skb,  
const struct net_device *in,  
const struct net_device *out,  
int (*okfn) (struct sk_buff *))  
{  
    __be32 sip,dip; 
 if(skb){  
   struct sk_buff *sb = NULL;  
   sb = skb;  
   struct iphdr *iph;  
   iph  = ip_hdr(sb);  
   sip = iph->saddr;  
   dip = iph->daddr;  
   //printk("Packet for source address: %d.%d.%d.%d\n destination address: %d.%d.%d.%d\n ", NIPQUAD(sip), NIPQUAD(dip));  
    }  
 return NF_ACCEPT;  
} 

int is_udp_local_packet(struct sk_buff *skb,struct net_device *in,unsigned short port)
{
    if(NULL ==  skb || NULL == in)
    {
        return 1;
    }    
    
    struct sk_buff *sb = skb;
    struct iphdr *iph = ip_hdr(sb);
    struct udphdr *uhdr = NULL;
    __be32 dip,sip;
    struct in_device *in_dev;
    struct in_ifaddr *ifa;
    dip = iph->daddr;
    sip = iph->saddr;
    
    if(IPPROTO_UDP == iph->protocol)
    {
        uhdr = udp_hdr(sb);
        rcu_read_lock();
        if ((in_dev = __in_dev_get_rcu(in)) == NULL) {
            rcu_read_unlock();
            return NF_ACCEPT;
        }

        for (ifa = in_dev->ifa_list; ifa; ifa = ifa->ifa_next) {
            if(ntohl(ifa->ifa_address) == ntohl(dip))
            {
                printk("device addr is 0x%x\r\n",ntohl(ifa->ifa_address));
                if(ntohs(uhdr->dest) == port)
                {
                    rcu_read_unlock();
                    return 0;
                }
            } 
        
        }

        rcu_read_unlock();
            
    }
    return 1;
}



static unsigned int local_in_func(unsigned int hooknum,struct sk_buff *skb,
const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff*))
{
    unsigned short local_port = 6789;
    if(0 == is_udp_local_packet(skb,in,local_port))
    {
        printk("recv port %d packet\r\n",local_port);
    }

    return NF_ACCEPT;
}
  
 struct nf_hook_ops sample_ops = {  
   .list =  {NULL,NULL},  
   .hook = sample,  
   .pf = PF_INET,  
   .hooknum = NF_INET_PRE_ROUTING,  
   .priority = NF_IP_PRI_FILTER+2  
 }; 

struct nf_hook_ops local_in_ops = {
    .list = {NULL,NULL},
    .hook = local_in_func,
    .pf = PF_INET,
    .hooknum = NF_INET_LOCAL_IN,
    .priority = NF_IP_PRI_FILTER+2

}; 
  
static int __init filter_init(void) {  
  nf_register_hook(&local_in_ops);
  nf_register_hook(&sample_ops); 
  wireway_dev_init(); 
  return 0;  
}  
  
  
static void __exit filter_exit(void) { 
    wireway_dev_exit(); 
  nf_unregister_hook(&sample_ops);
  nf_unregister_hook(&local_in_ops);  
}  
  
 module_init(filter_init);  
 module_exit(filter_exit);   
 MODULE_AUTHOR("chenkangrui");  
 MODULE_DESCRIPTION("netfilter_mod");  
