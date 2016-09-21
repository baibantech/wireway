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
#include <net/snmp.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <net/inet_ecn.h>
#include <linux/netfilter_ipv4.h>
#include <net/xfrm.h>
#include <linux/mroute.h>
#include <linux/netlink.h>


 
#include "wireway_dev.h" 
MODULE_LICENSE("GPL");  
DEFINE_PER_CPU(unsigned long long,timestamp_val);
int ip_rcv_finish_2(struct sk_buff *skb);

static unsigned long long rdtsc(void)
{
    unsigned int lo,hi;
    asm volatile
    (
     "rdtsc":"=a"(lo),"=d"(hi)
    );
    return (unsigned long long)hi<<32|lo;
}

void record_timestamp(void)
{
    unsigned long long tmp = 0;
    struct timeval stamp ;
    unsigned long jiff = 0;
    if(__get_cpu_var(timestamp_val) == 0)
    {
        __get_cpu_var(timestamp_val) = rdtsc();
        //printk("begin time stamp is %llx\r\n",__get_cpu_var(timestamp_val));
    }
    else
    {
        tmp = rdtsc();  
        //jiffies_to_timeval(clock_t_to_jiffies(tmp-__get_cpu_var(timestamp_val)),&stamp);
        //printk(" timestap is %u:%u\r\n",stamp.tv_sec,stamp.tv_usec);
        jiff = clock_t_to_jiffies(tmp-__get_cpu_var(timestamp_val)) ;
        printk("jiff is %llx\r\n",jiff);
        
        printk("ten msec is %d jiff \r\n",msecs_to_jiffies(10));
             
        printk("next hook time sub is %llx\r\n",tmp-__get_cpu_var(timestamp_val));
        __get_cpu_var(timestamp_val)= tmp;
    }


}


 
static unsigned int sample(  
unsigned int hooknum,  
struct sk_buff * skb,  
const struct net_device *in,  
const struct net_device *out,  
int (*okfn) (struct sk_buff *))  
{  
    __be32 sip,dip;
    struct timeval stamp ;

    struct udphdr *uhdr = NULL;
	unsigned long long tmp1 ,tmp2,tmp3 =0;
	tmp1= tmp2= tmp3 = 0;
	tmp2 = rdtsc();
	if(skb){ 
	 
   struct sk_buff *sb = NULL;  
   sb = skb;  
   struct iphdr *iph;  
   iph  = ip_hdr(sb);  
   sip = iph->saddr;  
   dip = iph->daddr;  
   //printk("Packet for source address: %d.%d.%d.%d\n destination address: %d.%d.%d.%d\n ", NIPQUAD(sip), NIPQUAD(dip));  
    if(IPPROTO_UDP == iph->protocol)
    {

        uhdr = udp_hdr(skb);
        if(ntohs(uhdr->dest) == 6789)
        {



	//tmp1 = rdtsc();
        //skb_get_timestamp(skb,&stamp);
        //__get_cpu_var(timestamp_val) = rdtsc();
	        skb->tstamp.tv64 = rdtsc();
            //record_timestamp();
	//tmp3 = rdtsc();
        //printk("pre routing skb timestap is %u:%u\r\n",stamp.tv_sec,stamp.tv_usec);  
    
            ip_rcv_finish_2(skb);
            return NF_STOLEN;
        }

        }
    

    }  
	//printk("hook time2 is %x,%x\r\n",rdtsc()-tmp2,tmp3-tmp1);
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
    char type;
    struct wireway_msg_head *p = NULL;
    struct udp_trans_wait_queue_head  *head = NULL;
    struct timeval stamp ;
    struct iphdr *iph;
	 struct udphdr *uhdr = NULL;

    iph  = ip_hdr(skb);

    if(IPPROTO_UDP == iph->protocol) 
    {
 	uhdr = udp_hdr(skb);
	if(ntohs(uhdr->dest) == local_port)
    {
	
    	printk("hook cost time is %x\r\n",rdtsc()- skb->tstamp.tv64);
        dump_stack();
    }

	#if 0
        skb_get_timestamp(skb,&stamp);
        record_timestamp();
        if(stamp.tv_sec != 0)
        {
            printk("local in pre skb timestap is %u:%u\r\n",stamp.tv_sec,stamp.tv_usec); 
            stamp = ktime_to_timeval(ktime_get_real());
            printk("local in over  skb timestap is %u:%u\r\n",stamp.tv_sec,stamp.tv_usec);
        }
	#endif
    }
	#if 0 
    if(0 == is_udp_local_packet(skb,in,local_port))
    {
        printk("recv port %d packet\r\n",local_port);
        #if 0
        __skb_pull(skb,sizeof(struct iphdr)+sizeof(struct udphdr));
        p = (struct wireway_msg_head*)(skb->data);
        printk("wireway_id is 0x%llx\r\n",p->wireway_id);
        printk("key  is 0x%llx\r\n",p->key);
        printk("msg type is %d\r\n",p->type);
        head = get_wait_queue_head(p->wireway_id,p->type);
        if(!head)
        {
           
        }
        else {
            udp_trans_wake(head,p->key,skb); 
        }
        #endif               
    }
	#endif
    return NF_ACCEPT;
}

static unsigned int local_out_func(unsigned int hooknum,struct sk_buff *skb,
const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff*))
{
   struct sk_buff *sb = NULL;
   sb = skb;
   struct iphdr *iph;
   iph  = ip_hdr(sb);
   //printk("Packet for source address: %d.%d.%d.%d\n destination address: %d.%d.%d.%d\n ", NIPQUAD(sip), NIPQUAD(dip));  
    if(IPPROTO_UDP == iph->protocol)
    {
        //tmp1 = rdtsc();
        //skb_get_timestamp(skb,&stamp);
        //__get_cpu_var(timestamp_val) = rdtsc();
        skb->tstamp.tv64 = rdtsc();
        //record_timestamp();
        //tmp3 = rdtsc();
        //printk("pre routing skb timestap is %u:%u\r\n",stamp.tv_sec,stamp.tv_usec);  
    }

    return NF_ACCEPT;
}

static unsigned int post_routing_func(unsigned int hooknum,struct sk_buff *skb,
const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff*))
{
	 struct sk_buff *sb = NULL;
   sb = skb;
   struct iphdr *iph;
   iph  = ip_hdr(sb);
   //printk("Packet for source address: %d.%d.%d.%d\n destination address: %d.%d.%d.%d\n ", NIPQUAD(sip), NIPQUAD(dip));  
    if(IPPROTO_UDP == iph->protocol)
    {
        //tmp1 = rdtsc();
        //skb_get_timestamp(skb,&stamp);
        //__get_cpu_var(timestamp_val) = rdtsc();
	printk("udp send hook time is %x\r\n",rdtsc()-skb->tstamp.tv64);	
        skb->tstamp.tv64 = rdtsc();
        //record_timestamp();
        //tmp3 = rdtsc();
        //printk("pre routing skb timestap is %u:%u\r\n",stamp.tv_sec,stamp.tv_usec);  
    }

    return NF_ACCEPT;
}

static inline bool ip_rcv_options(struct sk_buff *skb)
{
    struct ip_options *opt;
    const struct iphdr *iph;
    struct net_device *dev = skb->dev;

    /* It looks as overkill, because not all
       IP options require packet mangling.
       But it is the easiest for now, especially taking
       into account that combination of IP options
       and running sniffer is extremely rare condition.
                          --ANK (980813)
    */
    if (skb_cow(skb, skb_headroom(skb))) {
        IP_INC_STATS_BH(dev_net(dev), IPSTATS_MIB_INDISCARDS);
        goto drop;
    }

    iph = ip_hdr(skb);
    opt = &(IPCB(skb)->opt);
    opt->optlen = iph->ihl*4 - sizeof(struct iphdr);

    if (ip_options_compile(dev_net(dev), opt, skb)) {
        IP_INC_STATS_BH(dev_net(dev), IPSTATS_MIB_INHDRERRORS);
        goto drop;
    }

    if (unlikely(opt->srr)) {
        struct in_device *in_dev = __in_dev_get_rcu(dev);

        if (in_dev) {
            if (!IN_DEV_SOURCE_ROUTE(in_dev)) {
                if (IN_DEV_LOG_MARTIANS(in_dev))
                    net_info_ratelimited("source route option %pI4 -> %pI4\n",
                                 &iph->saddr,
                                 &iph->daddr);
                goto drop;
            }
        }

        if (ip_options_rcv_srr(skb))
            goto drop;
    }

    return false;
drop:
    return true;
}


int ip_rcv_finish_2(struct sk_buff *skb)
{
    const struct iphdr *iph = ip_hdr(skb);
    struct rtable *rt;
    unsigned long long tmp,tmp1,tmp2,tmp3;
    int res; 
    #if 0
    if (sysctl_ip_early_demux && !skb_dst(skb) && skb->sk == NULL) {
        const struct net_protocol *ipprot;
        int protocol = iph->protocol;

        ipprot = rcu_dereference(inet_protos[protocol]);
        if (ipprot && ipprot->early_demux) {
            ipprot->early_demux(skb);
            /* must reload iph, skb->head might have changed */
            iph = ip_hdr(skb);
        }
    }
    #endif
    /*
     *  Initialise the virtual path cache for the packet. It describes
     *  how the packet travels inside Linux networking.
     */
    if (!skb_dst(skb)) {
        tmp = rdtsc();
        int err = ip_route_input_noref(skb, iph->daddr, iph->saddr,
                           iph->tos, skb->dev);
        tmp1 = rdtsc();
        if (unlikely(err)) {
            if (err == -EXDEV)
                NET_INC_STATS_BH(dev_net(skb->dev),
                         LINUX_MIB_IPRPFILTER);
            goto drop;
        }
    }
#if 0
#ifdef CONFIG_IP_ROUTE_CLASSID
    if (unlikely(skb_dst(skb)->tclassid)) {
        struct ip_rt_acct *st = this_cpu_ptr(ip_rt_acct);
        u32 idx = skb_dst(skb)->tclassid;
        st[idx&0xFF].o_packets++;
        st[idx&0xFF].o_bytes += skb->len;
        st[(idx>>16)&0xFF].i_packets++;
        st[(idx>>16)&0xFF].i_bytes += skb->len;
    }
#endif
#endif
    if (iph->ihl > 5 && ip_rcv_options(skb))
        goto drop;
    rt = skb_rtable(skb);
    if (rt->rt_type == RTN_MULTICAST) {
        IP_UPD_PO_STATS_BH(dev_net(rt->dst.dev), IPSTATS_MIB_INMCAST,
                skb->len);
    } else if (rt->rt_type == RTN_BROADCAST)
        IP_UPD_PO_STATS_BH(dev_net(rt->dst.dev), IPSTATS_MIB_INBCAST,
                skb->len);
    res =  dst_input(skb);
    return res;
drop:
    kfree_skb(skb);
    return NET_RX_DROP;
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
 
struct nf_hook_ops local_out_ops = {
    .list = {NULL,NULL},
    .hook = local_out_func,
    .pf = PF_INET,
    .hooknum = NF_INET_LOCAL_OUT,
    .priority = NF_IP_PRI_FILTER+2

};
struct nf_hook_ops post_routing_ops = {
    .list = {NULL,NULL},
    .hook = post_routing_func,
    .pf = PF_INET,
    .hooknum = NF_INET_POST_ROUTING,
    .priority = NF_IP_PRI_FILTER+2

};
void list_nf_hook(int prot)
{
    struct nf_hook_ops *elem;
    int err;
    int hook_num = 0;

    for(hook_num = NF_INET_PRE_ROUTING ; hook_num < NF_INET_NUMHOOKS;hook_num++)
    {
        list_for_each_entry(elem, &nf_hooks[prot][hook_num], list) {
            printk("hooknum %d,hook func is %llx\r\n",hook_num,elem->hook);
        }
    }

    return ;

}

static int __init filter_init(void) {  
    __get_cpu_var(timestamp_val) = 0;
    nf_register_hook(&local_in_ops);
    nf_register_hook(&sample_ops);
    nf_register_hook(&local_out_ops);
    nf_register_hook(&post_routing_ops);
    list_nf_hook(PF_INET);
    //wireway_dev_init(); 
  return 0;  
}  
  
  
static void __exit filter_exit(void) { 
    //wireway_dev_exit();
  
    nf_unregister_hook(&post_routing_ops);
    nf_unregister_hook(&local_out_ops); 
    nf_unregister_hook(&sample_ops);
    nf_unregister_hook(&local_in_ops);  
}  
  
 module_init(filter_init);  
 module_exit(filter_exit);   
 MODULE_AUTHOR("lijiyong");  
 MODULE_DESCRIPTION("netfilter_mod");  
