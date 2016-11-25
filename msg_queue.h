#ifndef msg_queue_h
#define msg_queue_h
#include <stdio.h>
#include <sys/msg.h>
typedef key_t  que_handle;
#define OP_GET 1
#define OP_CREATE 2
#define IS_HANDLE_VALID(handle) (handle >=0)

#define INVALID_HANDLE (-1)

typedef struct msg_queue_dsc
{
    char name[32];
    int que_id;
    que_handle  queue_handle;
    int  queue_len;
    struct msg_queue_dsc *next;
}msg_queue_dsc;

typedef struct que_msg
{
    int msg_type;
    int mid_id;
    int msg_len;
    void *msg;
}que_msg;

int msg_queue_create(char *name,int len,char *flag);
int msg_queue_write(int queue_id,void *msg,int len);
int msg_queue_read(int queue_id,int flag,void *msg);
int msg_queue_cancel(char *name);

















#endif
