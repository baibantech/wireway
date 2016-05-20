    
#include <stdio.h>  
#include <stdlib.h>  
  
#include <string.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <netdb.h>  

#include "user_entity.h"
 
int port=6789;  
  
int main(int argc, char** argv) {  
  
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
