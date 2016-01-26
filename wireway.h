#ifndef __wireway__
#define __wireway__
#include <stdio.h>
#include "list.h"
#define MAX_LEN 32
#define point_in 1
#define point_out 2

/* point type define*/
#define point_peer 0
#define point_joint 1
#define point_bridge 2
#define point_bridge_peer 3
#define point_bridge_slave 4

/* wireway state define */
#define wire_init 1
#define wire_attaching 2
#define wire_active 3

/* point state define*/

#define point_idle 1
#define point_active 2


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
    int index;
    int  addr;
    struct location_tag *location;
    char dest;
    char state;
    char type;
}point;

#if 0
typedef struct bridge_slave_point_tag
{
    struct list_head brdige;
    


}
#endif


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

typedef struct point_desc
{
    int type;
    int index;
    union
    {
        struct point_block_peer
        {
            int type;
            int index;
            int state; 
            int dest;
            int addr;
        }peer;
        
        struct joint_block_peer
        {
            unsigned long next_wireway_id;
            int point_index;
        }jpeer;

        struct bridge_master
        {
            unsigned long peer_wireway_name_id;
            int peer_point_index;
        }bmaser;

        struct bridge_slave
        {
            unsigned long peer_wireway_name_id;
            int br_index;
        }bslave;
    }node_desc;

}point_desc_block;

typedef struct wireway_block
{
    int type;
    int point_num;
    int state;
    unsigned long name_id;
    unsigned long wireway_id;
    
    point_desc_block point_block[MAX_POINT_NUM_PER_BLOCK];
     
    union
    {
        unsigned long next_block_id;
        struct wireway_block *next_block_ptr;
    }next_block;

}wireway_block;

typedef struct wireway_tag
{
    char   *name;
    struct wireway_tag *next;
    
    struct point_tag *peer[2];
    int point_num;
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

