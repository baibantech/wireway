#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "user_entity.h"

int  get_entity_thread_num()
{
    return 1;
}
int get_wireway_thread_num()
{
    return 1;
}

void entity_thread_main()
{




}
void wireway_thread_main()
{
    int sin_len;
    int i = 0 ;
    int socket_descriptor;
    struct sockaddr_in sin;
    int message_len = sizeof(user_entity_content_block)+sizeof(entity_req);
    int ret = 0;
    char *message = malloc(message_len);
    if(!message)
    {
        return -1;
    }
    memset(message,0xFF,message_len);

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin.sin_port=htons(6789);
    sin_len=sizeof(sin);

    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));

    while(1)
    {
        ret = recvfrom(socket_descriptor,message,message_len,0,(struct sockaddr *)&sin,&sin_len);

        printf("recv msg\r\n");
        printf("msg Len is %d\r\n",ret);
        printf("receive from %s\r\n" , inet_ntoa(sin.sin_addr));
        
        while(i < 12)
        {
            if(0 == i%4)
            {
                printf("\r\n");
            }
            printf("0x%08x ",*(int*)message);
            message += sizeof(int);
            i++;
        }        

        memset(message,0,message_len);

    }
 
    close(socket_descriptor);
    exit(0);    

}

void entity_thread_init()
{
    pthread_t entity_tid;
    int i, ret = 0;
    int num = get_entity_thread_num();
    for(i = 0 ; i < num ; i++)
    {
        ret = pthread_create(&entity_tid,NULL,entity_thread_main,NULL);
        if(ret)
        {
            break;
        }
        
    }
    
}


void entity_test_main()
{
    int client_socket_fd = -1;
    struct sockaddr_in server_addr;
    entity_req *req = NULL;
    char *msg = NULL;
    int msg_len = 0;
    int ret = 0;
    int i = 0;
    int try_times = 3;
    memset(&server_addr, 0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6789);
    if(inet_pton(AF_INET,"192.168.112.138", &server_addr.sin_addr) == 0){
        perror("Server IP Address Error:");
        exit(1);
    }
 
    /* 创建socket */
    client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(client_socket_fd < 0)
    {
        printf("create client socket err\r\n");
        exit(1);
    }

    req = construct_req(pre_reg,"test1","test",strlen("hello pre reg"),"hello pre reg");
    if(!req)
    {
        pthread_exit(1);
    }
    
    msg_len = serial_entity_req(req,&msg);
    if(-1 == msg_len)
    {
        exit(1);
    }
    while(i < try_times) {
    if((ret =sendto(client_socket_fd, msg, msg_len,0,(struct sockaddr*)&server_addr,sizeof(server_addr))) < 0) 
    { 
        printf("Send File Name Failed:"); 
        exit(1); 
    } 
    printf("send msg byte %d\r\n",ret);
    i++;
    }
    close(client_socket_fd);
    while(1)
    {
        sleep(10);
    }
    
}


void entity_test_thread_init()
{
    pthread_t entity_tid;
    int ret = 0;
    ret = pthread_create(&entity_tid,NULL,entity_test_main,NULL);
    if(ret)
    {
        printf("create test thread err\r\n");
    }
}


void wireway_thread_init()
{
    pthread_t wireway_tid;
    int i,ret = 0;
    int num = get_wireway_thread_num();
    for(i = 0; i < num ; i++)
    {
        ret = pthread_create(&wireway_tid,NULL,wireway_thread_main,NULL);
        if(ret)
        {
            break;
        }
    }

}

