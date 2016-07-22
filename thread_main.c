#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "user_entity.h"

#ifdef __LITTILE_ENDIAN__
#define hton64
#define ntoh64
#else

#endif



int serial_entity_req(entity_req *req,char **msg)
{
    int name_len,group_name_len;
    int mem_len ;
    char *serial_mem = NULL;
    char *mem_tmp = NULL;
    if(!req || !msg)
    {
        return -1;
    }
    
    name_len = strlen(req->name);
    group_name_len = strlen(req->group_name);
    if(0 == name_len || 0 == group_name_len)
    {
        return -1;
    } 
    mem_len = name_len + group_name_len + req->msg_size + 4*sizeof(int) + sizeof(unsigned long);

    serial_mem = malloc(mem_len);
    if(NULL == serial_mem)
    {
        return -1;
    }    
    
    mem_tmp = serial_mem;
    
    *(int*)(mem_tmp) = req->msg_type;
    mem_tmp += sizeof(int);

    *(int*)(mem_tmp) = name_len;
    mem_tmp += sizeof(int);

    strcpy(mem_tmp,req->name);
    mem_tmp += name_len;

    *(int*)mem_tmp = group_name_len;
    mem_tmp += sizeof(int);

    strcpy(mem_tmp ,req->group_name);
    mem_tmp += group_name_len;

    *(unsigned long*)mem_tmp = req->reg_token;
    mem_tmp += sizeof(unsigned long);

    *(int*)mem_tmp = req->msg_size;
    mem_tmp +=  sizeof(int);
    
    memcpy(mem_tmp,req->content,req->msg_size);
    *msg = serial_mem;
    return mem_len;
}
entity_req *deserial_entity_req(char *serial_mem, int mem_len)
{
    int msg_type;
    unsigned long reg_token;
    char *name ,*group_name;
    int name_len,group_name_len;
    int msg_size;
    int offset = 0;
    entity_req *req = NULL;

    msg_type = *(int*)serial_mem;
    offset += sizeof(int);
    serial_mem += sizeof(int);
     
    name_len = *(int*)serial_mem;
    offset += sizeof(int);
    serial_mem += sizeof(int);

    name = malloc(name_len+1);
    if(!name)
    {
        return NULL;
    }
    strncpy(name,serial_mem,name_len);
    serial_mem += name_len;
    offset += name_len;
    
    group_name_len = *(int*)serial_mem;
    offset += sizeof(int);
    serial_mem += sizeof(int);
    
    group_name = malloc(group_name_len +1);
    if(!group_name)
    {
        free(name);
        return NULL;
    }
    
    strncpy(group_name,serial_mem,group_name_len);
    serial_mem += group_name_len;
    offset += group_name_len;

    reg_token = *(unsigned long*)serial_mem;
    serial_mem += sizeof(unsigned long);
    offset += sizeof(unsigned long);
    
    msg_size = *(int*)serial_mem;
    offset += sizeof(int);
    serial_mem += sizeof(int);

    if((mem_len - offset) != msg_size)
    {
        free(name);
        free(group_name);
        return NULL;
    }
    
    req = malloc(sizeof(entity_req)+msg_size);
    if(!req)
    {
        free(name);
        free(group_name);
        return NULL;
    }

    
    req->msg_type = msg_type;
    req->name = name;
    req->group_name = group_name;
    req->reg_token = reg_token;
    req->msg_size = msg_size;
    memcpy(req->content,serial_mem,msg_size);
        
    print_entity_req(req);

    return req;
}


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
    entity_req *req;
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
        
        req = deserial_entity_req(message,ret);
        free(req);
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
    int try_times = 1;
    memset(&server_addr, 0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6789);
    if(inet_pton(AF_INET,"127.0.0.1", &server_addr.sin_addr) == 0){
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

