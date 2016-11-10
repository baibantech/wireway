    
#include <stdio.h>  
#include <stdlib.h>  
#include <error.h>  
#include <string.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <netdb.h>  
#include <fcntl.h>
#include "user_entity.h"
#include <sys/ioctl.h> 
#define MERGE_IOCTL_MAGIC 0xF6


#define MERGE_IOC_SHOW_VMA _IO(MERGE_IOCTL_MAGIC,1) 

int port=6789; 

int init_deamon()
{
    int pid;
    if(pid = fork())
    {
        exit(0);
    }
    else if(pid < 0)
    {
        exit(1);
    }

    setsid();
    printf("deamon process run\r\n");

} 
  
int main(int argc,char** argv)
{
    int fd  = 0;
    init_deamon();
   
    #if 0
     printf("init socket \r\n");
    wireway_thread_init();
    sleep(10);
    entity_test_thread_init();
    #endif
    fd = open("/dev/merge_mem",O_RDWR);
    if(fd == -1)
    {
        perror("open");
        printf("err open dev\r\n");
    }     
    ioctl(fd,MERGE_IOC_SHOW_VMA ,NULL);
    close(fd);
    while(1)
    {
        sleep(1);
    }
       
    exit(0);
    return 0;
}

int init_server_socket() {  
  
    int sin_len;  
  
    int socket_descriptor;  
    struct sockaddr_in sin;  
    int message_len = sizeof(user_entity_content_block)+sizeof(entity_req);
    char *message = malloc(message_len);
    if(!message)
    {
        return -1;
    }    
    memset(message,0,message_len);
 
    bzero(&sin,sizeof(sin));  
    sin.sin_family=AF_INET;  
    sin.sin_addr.s_addr=htonl(INADDR_ANY);  
    sin.sin_port=htons(port);  
    sin_len=sizeof(sin);  
  
    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);  
    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));  
  
    while(1)  
    {  
        recvfrom(socket_descriptor,message,sizeof(message),0,(struct sockaddr *)&sin,&sin_len);  

        printf("Response from server:%s\n",message); 
 
    }  
  
    close(socket_descriptor);  
    exit(0);  
  
    return (EXIT_SUCCESS); 
} 
