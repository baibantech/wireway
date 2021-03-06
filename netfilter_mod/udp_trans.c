#include <linux/kernel.h>
#include <asm-generic/unaligned.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <net/icmp.h>
#include <net/inet_hashtables.h>
#include <net/route.h>
#include <net/ip.h>
#include "wireway_dev.h"
#define udp_trans_wait_time 0x0FFFF

unsigned long wireway_msg_key_num = 0;


struct udp_trans_waitor
{
    struct list_head waitor_list;
    unsigned long  key;
    void *reply;
    void *priv;  
};

struct udp_trans_wait_queue_head wait_queue = {
    .lock = __SPIN_LOCK_UNLOCKED(wait_queue.lock),
    .waitor_list = {&wait_queue.waitor_list,&wait_queue.waitor_list}
};
struct udp_trans_wait_queue_head* get_wait_queue_head(unsigned long wireway_id,char type)
{
    return &wait_queue;
}



void* udp_trans_wait(struct udp_trans_wait_queue_head *wait_head,int key,unsigned long time_out)
{
    unsigned long flags;
    struct udp_trans_waitor waitor;
    struct udp_trans_waitor *pos = NULL;
    int time  = 0;
    INIT_LIST_HEAD(&waitor.waitor_list);
    waitor.key = key;
    waitor.priv = (void*)current;
    waitor.reply = NULL;
    spin_lock_irqsave(&wait_head->lock,flags);
    list_for_each_entry(pos,&wait_head->waitor_list,waitor_list) /*if list is null do not entor loop*/
    {
        if(pos->key < key)
        {
            continue;
        }
        break;
    }
        
    list_add(&waitor.waitor_list,&pos->waitor_list);
    set_current_state(TASK_UNINTERRUPTIBLE);
    spin_unlock_irqrestore(&wait_head->lock,flags);
    time = schedule_timeout(time_out);
    
    spin_lock_irqsave(&wait_head->lock,flags);
    if(list_empty_careful(&waitor.waitor_list))
    {
        list_del_init(&waitor.waitor_list);
    }
    spin_unlock_irqrestore(&wait_head->lock,flags);
     
    return waitor.reply;
}

int udp_trans_wake(struct udp_trans_wait_queue_head *head,    unsigned long key,void  *msg)
{
    unsigned long flags;
    struct udp_trans_waitor *pos = NULL;

    spin_lock_irqsave(&head->lock,flags);
    list_for_each_entry(pos,&head->waitor_list,waitor_list) /*if list is null do not entor loop*/
    {
        printk("get the wait task key is %d\r\n",key);
        if(pos->key == key)
        {
            pos->reply = msg;
            wake_up_process(pos->priv);
            spin_unlock_irqrestore(&head->lock,flags);
            return 0;
        }
    }
    spin_unlock_irqrestore(&head->lock,flags);
    return -1;
}



extern struct net init_net; //sock kern net namespace
struct socket *sock = NULL;
void print_sock_info( struct sock *sk)
{
    struct inet_sock *inet = inet_sk(sk);
    printk("sk->sk_bound_dev_if is 0x%x\r\n",sk->sk_bound_dev_if);
    printk("inet->inet_saddr is 0x%x\r\n",inet->inet_saddr);
    printk("inet->inet_rcv_saddr is 0x%x\r\n",inet->inet_rcv_saddr);
    printk("inet->inet_num is 0x%x\r\n",inet->inet_num);    
    printk("inet->tos is 0x%x\r\n",inet->tos);
    printk("inet->uc_index is 0x%x\r\n",inet->uc_index);
    return;
}

int kernel_send_msg(struct socket *sock, struct sockaddr *addr, int addr_len,const char *buffer, const size_t length)
{
    struct msghdr   msg = {.msg_flags = MSG_DONTWAIT|MSG_NOSIGNAL};
    struct kvec iov;
    int     len;
    
    msg.msg_name = addr;
    msg.msg_namelen = addr_len;

    iov.iov_base     = (void *)buffer;
    iov.iov_len      = length;
    len = kernel_sendmsg(sock, &msg, &iov, 1, (size_t)(length));

    return len;
}
void kernel_udp_sock_test(struct net *net)
{
    struct socket *sock;
    int result;
    struct sock *sk;
    struct sockaddr_in addr;
    char *buf = "hello test kernel sock\r\n";
    int count = 0;
    if(NULL == net)
    {
        net = &init_net;
    }

    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6789);
    addr.sin_addr.s_addr = in_aton("192.168.1.112");

    result = sock_create_kern(PF_INET,SOCK_DGRAM,IPPROTO_UDP,&sock);
    if(result < 0)
    {
        printk("create socket err\r\n");
        return;
    }
    
    sk = sock->sk;
    print_sock_info(sk);
    printk("\r\n\r\n");

    //sk->sk_prot->get_port(sk,0);
    count = kernel_send_msg(sock,&addr,sizeof(addr),buf,sizeof(buf));
    printk("sizeof buf is %d\r\n",sizeof(buf));
    printk("send msg buf is %d\r\n",count);
    printk("\r\n\r\n");
    print_sock_info(sk);
    return;
}

void kernel_udp_sock_realse(void)
{
    if(sock)
    {
        sk_release_kernel(sock->sk);
    }
}

/*net arg from user space,if null init_net*/
int send_udp_packet_no_fagment(struct net *net,int dst_ip,short dst_port ,char *msg,int len)
{
    struct rtable *rt = NULL;
    struct flowi4 fl4_stack;
    struct flowi4 *fl4 ;
    struct sk_buff *skb;  
    int total_len, eth_len, ip_len, udp_len, header_len;
    int hh_len ;
    int mtu;  
    struct udphdr *udph;  
    struct iphdr *iph;   
    int local_ip;  
    struct net_device *dev;
    short local_port = 6789;
    struct wireway_msg_head *w_head = NULL;
    
    if(NULL == net)
    {
        net = &init_net;
    }
 
    fl4 = &fl4_stack;
    flowi4_init_output(fl4, 0, 0, 0,
               RT_SCOPE_UNIVERSE, IPPROTO_UDP,
               0,  dst_ip, 0, dst_port, htons(6789));

    rt = __ip_route_output_key(net, fl4);
    if (IS_ERR(rt)) {
        return -1;        
    }
    local_ip = ntohl(fl4->saddr);
    dev = rt->dst.dev;
    //printk("loca_ip is 0x%x\r\n",local_ip);
    hh_len = LL_RESERVED_SPACE(dev); 
    // 设置各个协议数据长度  
    len +=  sizeof(struct wireway_msg_head);
    udp_len = len + sizeof(*udph);  
    ip_len = eth_len = udp_len + sizeof(*iph);  
    total_len = eth_len + hh_len;  
    header_len = total_len - len ;

//    printk("ETH_HLEN is %d,NET_IP_ALIGN is %d\r\n",ETH_HLEN,NET_IP_ALIGN);

  //  printk("net device ll head len is %d\r\n",LL_RESERVED_SPACE(dev));
    
   // printk("rt dst header len is %d\r\n",rt->dst.header_len);
   // printk("LL_MAX_HEADER is %d\r\n",LL_MAX_HEADER);
   // printk("msg head len is %d\r\n",sizeof(struct wireway_msg_head));
    // 分配skb  
    skb = alloc_skb( total_len + 15, GFP_ATOMIC );  
    if ( !skb ) {  
        printk( "alloc_skb fail./n" );  
        return -1 ;  
    }  
  
    // 预先保留skb的协议首部长度大小  
    skb_reserve( skb, 15 + header_len );  
  
    // 拷贝负载数据  
    skb_copy_to_linear_data_offset(skb,sizeof(struct wireway_msg_head),msg, len- sizeof(struct wireway_msg_head));  
    skb->len += len;  
    w_head = (struct wireway_msg_head *)skb->data;
    w_head->type = wireway_msg_data;
    w_head->key = wireway_msg_key_num++;
    w_head->wireway_id = 1; 
    // skb->data 移动到udp首部  
    skb_push(skb, sizeof(*udph));  
    skb_reset_transport_header(skb);  
    udph = udp_hdr(skb);  
    udph->source = htons(local_port);  
    udph->dest = htons(dst_port);  
    udph->len = htons(udp_len);  
    udph->check = 0;  
    udph->check = csum_tcpudp_magic(htonl(local_ip),  
            htonl(dst_ip),  
            udp_len, IPPROTO_UDP,  
            csum_partial(udph, udp_len, 0));  
    if (udph->check == 0)  
        udph->check = CSUM_MANGLED_0;  
     
    // skb->data 移动到ip首部  
    skb_push(skb, sizeof(*iph));  
    skb_reset_network_header(skb);  
    iph = ip_hdr(skb);  
  
    /* iph->version = 4; iph->ihl = 5; */  
    put_unaligned(0x45, (unsigned char *)iph);  
    iph->tos      = 0;  
    put_unaligned(htons(ip_len), &(iph->tot_len));  
    iph->id       = 0;  
    iph->frag_off = 0;  
    iph->ttl      = 64;  
    iph->protocol = IPPROTO_UDP;  
    iph->check    = 0;  
    put_unaligned(htonl(local_ip), &(iph->saddr));  
    put_unaligned(htonl(dst_ip), &(iph->daddr));  
    iph->check    = ip_fast_csum((unsigned char *)iph, iph->ihl);  
    
    skb_dst_set(skb, &rt->dst); 
    
    iph->tot_len = htons(skb->len);

    skb->dev = skb_dst(skb)->dev;
    skb->protocol = htons(ETH_P_IP);
    ip_local_out_sk(NULL,skb);
 
    udp_trans_wait(&wait_queue,w_head->key,10000);       
     
    return ;  
}
int send_udp_packet_test(void)
{
    int count ;
    char *buf = "hello world\r\n";
    int dst_ip = 0xC0A8016B;   
    count = send_udp_packet_no_fagment(NULL, dst_ip,6789 ,buf,strlen(buf));
    return count;
}
