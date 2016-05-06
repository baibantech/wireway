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
#define wire_restoring 4

/* point state define*/

#define point_idle 1
#define point_active 2

#define MAX_POINT_NUM_PER_BLOCK 12


#define user_entity_pre_reg 1
#define user_entity_reg 2
/* define user entity desc*/
typedef struct user_port_addr
{
    int addr_type;
    struct list_head list;
    union {
        unsigned long addr;
        char addr_string[64];
    }port_logic_addr;

    union {
        unsigned long phy_addr;
        char phy_addr_string[64];
    }port_phy_addr;

}user_port_addr;

typedef struct user_logic_port_tag
{
    struct list_head list;
    struct list_head addr_list;
    char port_name[32];
    int port_index;
    int port_type;
    int port_state;
    int addr_num;
}user_logic_port;

typedef struct user_entity_desc_tag
{
    char state;
    char *name;
    char *group_name;
    unsigned long name_id;
    unsigned long root_block_id;
    unsigned long user_token;
    int port_num;
    struct list_head port_list;

    int relate_wireway_num;
    int relate_point_num;
    struct list_head create_wire_list;
    struct list_head attach_point;
    user_entity_root_block *block;

}user_entity_desc;
/*end define user entity desc*/

typedef struct point_location_tag
{
    struct list_head usr_list;
    int user_port_index;
}point_location;

typedef struct point_tag
{
    struct list_head list;
    struct list_head fib;
    struct list_head usr;
    struct wireway_tag *wire;
    int index;
    char dest;
    char state;
    char type;
    point_location loaddr;
}point;

typedef struct bridge_slave_point_tag
{
    struct list_head list;
    struct wireway_tag *wire;
    char type;
    int point_index;
    int index;
}bridge_slave;


typedef struct bridge_point_tag
{
    struct list_head list;
    struct wireway_tag *wire;
    struct bridge_slave_point_tag bridge_slave;
    int location_peer_index;
    int slave_point_index;
    point_location loaddr;
    int index;
    char dest;
    char type;    
}bridge_point;

   // struct list_head bridge;

typedef struct joint_point_tag
{
    struct list_head list;
    int index;
    char type;
}joint_point;

typedef struct point_desc_block
{
    char type;
    int index;
    union
    {
        struct point_block_peer
        {
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
            char dest;
            int location_peer_index;
            int slave_point_index;
            unsigned long peer_wireway_name_id;
        }bmaster;

        struct bridge_slave
        {
            unsigned long master_wireway_name_id;
            int bridge_index;
            int point_index;
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
    user_entity_desc *ower;
    struct list_head usr_list; 
    
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

