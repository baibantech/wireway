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
#include <net/udp.h>
#include <net/inet_hashtables.h>
 
#include "wireway_dev.h" 
MODULE_LICENSE("GPL");  
<<<<<<< HEAD
DEFINE_PER_CPU(unsigned long long,timestamp_val);

DEFINE_PER_CPU(unsigned long long,timestamp_val1);
int ip_rcv_finish_2(struct sk_buff *skb);
struct udp_table *tmp_table =(struct udp_table*) (void*)(unsigned long long)0xffffffff81d18e70ULL;
=======
int msg_rcv = 0;
int msg_in_err = 0;
int is_udp_local_packet(struct sk_buff *skb,struct net_device *in,unsigned short port);
extern void show_packet_rcv_control(void);
int irq_cpu = 3;
unsigned long msg_in_queue_av = 0;
unsigned long msg_in_queue_max = 0;
unsigned long msg_in_queue_step1 = 0;
unsigned long msg_in_queue_step2 = 0;
unsigned long wake_av = 0;
unsigned long wake_max = 0;
unsigned long wake_total = 0;
unsigned long msg_in_queue_total = 0;
extern unsigned long inq_step1 ;
extern unsigned long inq_step2 ;
>>>>>>> 7771209cf28b70af4f55b06b0aa788e50ab11d13

static unsigned long long rdtsc(void)
{
    unsigned int lo,hi;
    asm volatile
    (
<<<<<<< HEAD
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


 
=======
     "rdtsc":"=a"(lo),"=d"(hi) 
    ); 
    return (unsigned long long)hi<<32|lo;
}   

  
>>>>>>> 7771209cf28b70af4f55b06b0aa788e50ab11d13
static unsigned int sample(  
unsigned int hooknum,  
struct sk_buff * skb,  
const struct net_device *in,  
const struct net_device *out,  
int (*okfn) (struct sk_buff *))  
{  
    __be32 sip,dip;
<<<<<<< HEAD
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
=======
    int ret;
    unsigned long begin;
    unsigned long end;
    unsigned long  flag;        
    if(smp_processor_id() != irq_cpu)
    {
        printk("irq cpu is %d\r\n",smp_processor_id());
        irq_cpu = smp_processor_id();
    } 
    if(skb){  
        struct sk_buff *sb = NULL;  
        sb = skb;  
        struct iphdr *iph;  
        iph  = ip_hdr(sb);  
        sip = iph->saddr;  
        dip = iph->daddr;  
        
        if(0 == is_udp_local_packet(skb,in,6789))
        {
            if(collector_main)
            {   
                msg_rcv++;
                __skb_pull(skb,sizeof(struct iphdr)+sizeof(struct udphdr));
                local_irq_save(flag);
                begin = rdtsc();
                //local_irq_save(flag);
                ret = lfrwq_inq(collector_main->rcv_queue,skb);
                //local_irq_restore(flag);
                end =  rdtsc();
                local_irq_restore(flag);
                
                msg_in_queue_total += end-begin;
                msg_in_queue_av = msg_in_queue_total/msg_rcv;
                
                if(end- begin > msg_in_queue_max)
                {
                    msg_in_queue_max = end -begin;
                    msg_in_queue_step1 = inq_step1;
                    msg_in_queue_step2 = inq_step2;
                }
                if(0 == ret)
                {
                    #if 1
                    begin = rdtsc();
                    packet_rcv_wakeup();
                    end = rdtsc(); 
                    
                    wake_total += end - begin ;
                    wake_av = wake_total/msg_rcv;
                    if(wake_max < end -begin)
                    {
                        wake_max = end -begin;
                    }                   
                    #endif

 
                    return NF_STOLEN;
                }
                else
                {
                    packet_rcv_wakeup();
                    show_packet_rcv_control();
                }
            }
        } 
	return NF_DROP;
    }  
    return NF_ACCEPT;  
>>>>>>> 7771209cf28b70af4f55b06b0aa788e50ab11d13
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
	printk("route cost time %x,%x\r\n",__get_cpu_var(timestamp_val),__get_cpu_var(timestamp_val1));	
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

static inline struct sock *__udp4_lib_lookup_skb(struct sk_buff *skb,

						 __be16 sport, __be16 dport,

						 struct udp_table *udptable)

{

	const struct iphdr *iph = ip_hdr(skb);



	return __udp4_lib_lookup(dev_net(skb_dst(skb)->dev), iph->saddr, sport,

				 iph->daddr, dport, inet_iif(skb),

				 udptable);

}

static unsigned int udp4_portaddr_hash(struct net *net, __be32 saddr,

				       unsigned int port)

{

	return jhash_1word((__force u32)saddr, net_hash_mix(net)) ^ port;

}

static struct sock *__udp4_lib_demux_lookup(struct net *net,

					    __be16 loc_port, __be32 loc_addr,

					    __be16 rmt_port, __be32 rmt_addr,

					    int dif)

{

	struct sock *sk, *result;

	struct hlist_nulls_node *node;

	unsigned short hnum = ntohs(loc_port);

	unsigned int hash2 = udp4_portaddr_hash(net, loc_addr, hnum);

	unsigned int slot2 = hash2 & (udp_table.mask);

	struct udp_hslot *hslot2 = &udp_table.hash2[slot2];

	INET_ADDR_COOKIE(acookie, rmt_addr, loc_addr);

	const __portpair ports = INET_COMBINED_PORTS(rmt_port, hnum);



	rcu_read_lock();

	result = NULL;

	udp_portaddr_for_each_entry_rcu(sk, node, &hslot2->head) {

		if (INET_MATCH(sk, net, acookie,

			       rmt_addr, loc_addr, ports, dif))

			result = sk;

		/* Only check first socket in chain */

		break;

	}



	if (result) {

		if (unlikely(!atomic_inc_not_zero_hint(&result->sk_refcnt, 2)))

			result = NULL;

		else if (unlikely(!INET_MATCH(sk, net, acookie,

					      rmt_addr, loc_addr,

					      ports, dif))) {

			sock_put(result);

			result = NULL;

		}

	}

	rcu_read_unlock();

	return result;

}



void udp_v4_early_demux(struct sk_buff *skb)

{

	struct net *net = dev_net(skb->dev);

	const struct iphdr *iph;

	const struct udphdr *uh;

	struct sock *sk,*sk1;

	struct dst_entry *dst;

	int dif = skb->dev->ifindex;



	/* validate the packet */

	if (!pskb_may_pull(skb, skb_transport_offset(skb) + sizeof(struct udphdr)))

		return;



	iph = ip_hdr(skb);

	uh = udp_hdr(skb);


	#if 0
	if (skb->pkt_type == PACKET_BROADCAST ||

	    skb->pkt_type == PACKET_MULTICAST)

		sk = __udp4_lib_mcast_demux_lookup(net, uh->dest, iph->daddr,
uh->source, iph->saddr, dif);
	#endif
	 if (skb->pkt_type == PACKET_HOST)

		sk = __udp4_lib_demux_lookup(net, uh->dest, iph->daddr,

					     uh->source, iph->saddr, dif);

	else

		return;
	
	//sk1 = __udp4_lib_lookup_skb(skb,uh->source,uh->dest,&udp_table);


	if (!sk)
	{
		printk("not search sock\r\n");
		return;
	}
	//printk("sk1 is %llx\r\n",sk1);

	skb->sk = sk;

	skb->destructor = sock_edemux;

	dst = sk->sk_rx_dst;



	if (dst)

		dst = dst_check(dst, 0);

	if (dst)

		skb_dst_set_noref(skb, dst);

}







ip_rcv_finish_2(struct sk_buff *skb)
{
    const struct iphdr *iph = ip_hdr(skb);
    struct rtable *rt;
    unsigned long long tmp,tmp1,tmp2,tmp3;
    int res;
	struct sock *sk1;
	 const struct udphdr *uh;
	uh = udp_hdr(skb);
	tmp = tmp1=tmp2=tmp3 = 0; 
    #if 1
	printk("skb->pkt_type is %d\r\n",skb->pkt_type);
    tmp2 = rdtsc();
    if (sysctl_ip_early_demux && !skb_dst(skb) && skb->sk == NULL) {
        const struct net_protocol *ipprot;
        int protocol = iph->protocol;

        ipprot = (struct net_protocol*)(void *)*(unsigned long long *)((unsigned long long)(0xffffffff81d184e0 +sizeof(void*)*protocol));
        if (ipprot && ipprot->early_demux) {
	//	printk("early_demux is %llx\r\n",ipprot->early_demux);
          //  ipprot->early_demux(skb);
            /* must reload iph, skb->head might have changed */
	 udp_v4_early_demux(skb);
            iph = ip_hdr(skb);
        }
    }
	tmp3 = rdtsc();

	printk("skb sk is %llx\r\n",skb->sk);
    #endif
    /*
     *  Initialise the virtual path cache for the packet. It describes
     *  how the packet travels inside Linux networking.
     */

	printk("udp source is %d,desr is %d\r\n",ntohs(uh->source),ntohs(uh->dest));
	
	sk1 = __udp4_lib_lookup_skb(skb,uh->source,uh->dest,&udp_table);

	#if 0
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
	__get_cpu_var(timestamp_val) = tmp3-tmp2;
	__get_cpu_var(timestamp_val1)  = tmp1-tmp;
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
	#endif
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
<<<<<<< HEAD
    __get_cpu_var(timestamp_val) = 0;
    nf_register_hook(&local_in_ops);
    nf_register_hook(&sample_ops);
    nf_register_hook(&local_out_ops);
    nf_register_hook(&post_routing_ops);
    list_nf_hook(PF_INET);
    //wireway_dev_init(); 
  return 0;  
=======
    nf_register_hook(&local_in_ops);
    nf_register_hook(&sample_ops); 
    wireway_dev_init(); 
    return 0;  
>>>>>>> 7771209cf28b70af4f55b06b0aa788e50ab11d13
}  
  
  
static void __exit filter_exit(void) { 
<<<<<<< HEAD
    //wireway_dev_exit();
  
    nf_unregister_hook(&post_routing_ops);
    nf_unregister_hook(&local_out_ops); 
=======
    wireway_dev_exit(); 
>>>>>>> 7771209cf28b70af4f55b06b0aa788e50ab11d13
    nf_unregister_hook(&sample_ops);
    nf_unregister_hook(&local_in_ops);  
}  
  
 module_init(filter_init);  
 module_exit(filter_exit);   
 MODULE_AUTHOR("lijiyong");  
 MODULE_DESCRIPTION("netfilter_mod");  
