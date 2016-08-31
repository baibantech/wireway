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
    struct udphdr *udph;  
    struct iphdr *iph;   
    int local_ip;  
    short local_port = 6789;
    
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
    printk("loca_ip is 0x%x\r\n",local_ip); 
    // 设置各个协议数据长度  
    udp_len = len + sizeof(*udph);  
    ip_len = eth_len = udp_len + sizeof(*iph);  
    total_len = eth_len + ETH_HLEN + NET_IP_ALIGN;  
    header_len = total_len - len;
    // 分配skb  
    skb = alloc_skb( total_len + LL_MAX_HEADER, GFP_ATOMIC );  
    if ( !skb ) {  
        printk( "alloc_skb fail./n" );  
        return -1 ;  
    }  
  
    // 预先保留skb的协议首部长度大小  
    skb_reserve( skb, LL_MAX_HEADER + header_len );  
  
    // 拷贝负载数据  
    skb_copy_to_linear_data(skb, msg, len);  
    skb->len += len;  
  
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
    //ip_finish_output2(skb); 

    /* 
    // skb->data 移动到eth首部 
     eth = (struct ethhdr *) skb_push(skb, ETH_HLEN); 
    skb_reset_mac_header(skb); 
    skb->protocol = eth->h_proto = htons(ETH_P_IP); 
    memcpy(eth->h_source, dev_addr, ETH_ALEN); 
    memcpy(eth->h_dest, remote_mac, ETH_ALEN); 
     
    */  
       
  
free_skb:  
    printk( "free skb./n" );  
    //kfree_skb(skb);  
    return ;  
   
}
int send_udp_packet_test(void)
{
    int count ;
    char *buf = "hello world\r\n";
    int dst_ip = 0xC0A80170;   
    count = send_udp_packet_no_fagment(NULL, dst_ip,6789 ,buf,strlen(buf));
    return count;
}
