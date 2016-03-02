#ifndef __user_entity__
#define __user_entity__
#include "wireway.h"

// define user entity state

#define user_entity_pre_reg 1
#define user_entity_reg 2


typedef struct relate_point_desc
{
    int index;
    int port_index;
    point *p;
}relate_wireway_point;

typedef struct relate_point_array
{
    int num;
    relate_wireway_point[100];
    relate_point_array *next_array;
}relate_point_array;
typedef struct user_port_addr
{
    union {
        unsigned long addr;
        char addr_string[64];
    }port_logic_addr;
    
    union {
        unsigned long phy_addr;
        char phy_addr_string[64];
    }port_phy_addr;    

    struct user_port_addr *next;
}user_port_addr;

typedef struct user_logic_port_tag
{
    char port_name[32];
    int port_index;
    int port_type;
    int port_state;
    int addr_num;
    user_port_addr *port_addr;
}user_logic_port;

typedef struct user_entity_desc_tag
{
    char state;
    char *name;
    char *group_name;
    unsigned long name_id;
    unsigned long user_block_id;
    unsigned long user_token;
    int port_num;    
    user_logic_port *port;
    relate_point_array *owner_point;
    relate_point_array *attach_point;   
}user_entity_desc;

typedef struct user_entity_block
{
    unsigned long next_block_id;
    int content_len;
    int base_offset;
    int port_offset;
    int port_len;
    int relate_point_offset;
    int relate_point_len;
    char content[500];
}user_entity_block;

//int lookup_resource_server();
//int reg_to_resource_server(); 
int prepare_reg_entity();
int reg_entity();
int addr_entity_port();

#endif
