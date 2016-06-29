#include "msg_queue.h"
msg_queue_dsc *g_pstMsgQueuehead = NULL;
int  msg_queue_create(char *name,int len,char *flag)
{
    msg_queue_dsc *ptr ,*tmp ;
    que_handle handle ;
    int i = 0;
    if(g_pstMsgQueuehead)
    {
        ptr = tmp = g_pstMsgQueuehead;
        while(tmp)
        {
            ptr = tmp;
            if(0 == strcmp(tmp->name,name))
            {
                *flag = OP_GET;
                return tmp->que_id;
            }
            tmp = tmp->next;
            i++;
            if(i > 255)
            {
                return -1;
            }
        }
    }
        
    tmp = malloc(sizeof(msg_queue_dsc));
    if(tmp)
    {
        strcpy(tmp->name,name);
        handle =  ftok(".",i);
        if(!IS_HANDLE_VALID(handle))
        {
            free(tmp);
            return -1;
        }
        tmp->queue_handle = handle;
        tmp->queue_len = len;
        tmp->que_id = msgget(handle,IPC_CREAT|0666);
        if(-1 != tmp->que_id)
        {
            free(tmp);
            return -1;
        }

    }
    else
    {
        return -1;
    }

    if(ptr)
    {
        ptr->next = tmp;
        tmp->next = NULL;
    }        
    else
    {
        g_pstMsgQueuehead = tmp;
        tmp->next = NULL;
    }
    return tmp->que_id; 
}
int msg_queue_write(int que_id,void *msg,int len)
{
    return msgsnd(que_id,msg,len,0); 
}
int msg_queue_read(int que_id,int flag ,void *msg)
{
   return msgrcv(que_id,msg,sizeof(que_msg),0,0);
}

