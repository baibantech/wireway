#ifndef __wireway__
#define __wireway__
#include <stdio.h>
#include "list.h"
#define MAX_LEN 32
#define point_in 1
#define point_out 2


#define point_peer 0
#define point_joint 1
#define point_bridge 2
#define point_bridge_peer 3
#define point_bridge_slave 4

#define wire_init 1
#define wire_attaching 2
#define wire_active 3

#define MAX_POINT_NUM_PER_BLOCK 12
typedef struct location_tag
{
    int uid;
    int addr;
    int prior;
    int type;
}location;

typedef struct point_tag
{
    struct list_head list;
    struct list_head fib;
    struct wireway_tag *wire;
    int  addr;
    struct location_tag *location;
    char dest;
    char state;
    char type;
}point;

typedef struct bridge_point_tag
{
    struct list_head list;
    struct list_head bridge;
    struct wireway_tag *wire;
    struct point_tag *peer;
    struct point_tag *location_peer;
    char dest;
    char type;    
}bridge_point;

typedef struct joint_point_tag
{
    struct list_head list;
    char type;
}joint_point;

typedef struct point_desc_block
{
    int type;
    int state; 
    int dest;
    int addr;
    unsigned long location_id;
    unsigned long next_point_id;    
}point_desc_block;
        

typedef struct bridge_desc_block
{
    int type;
    unsigned long wireway_id;
        

    

}bridge_desc_block;

typedef struct bridge_point_desc
{
    unsigned long slave_wireway_id;
    int insert_index;
    
};


typedef struct wireway_block
{
    int type;
    int point_num;
    int state;
    unsigned long name_id;
    unsigned long wireway_id;
    
    point_desc_block point_block[MAX_POINT_NUM_PER_BLOCK];
    unsigned long next_block_id; 
     
}wireway_block;

typedef struct wireway_tag
{
    char   *name;
    struct wireway_tag *next;
    
    struct point_tag *peer[2];
    struct list_head point_list;
    wireway_block *block;
    int state;
}wireway;

typedef struct wireway_fib_tag
{
    struct list_head list;
    int src;
    int dst;
}wireway_fib;


#endif

