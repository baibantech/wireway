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

    memset(&server_addr, 0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.112.138");//local ip 
    server_addr.sin_port = htons(6789);

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
        exit(1);
    }
    
    msg = serial_entity_req(req);

    return client_socket_fd;
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

