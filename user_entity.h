#ifndef __user_entity__
#define __user_entity__

#define USER_BLOCK_CONTENT_LEN 1024
#define USER_BLOCK_MAX_STORAGE_ARRAY (1024-sizeof(int)*6-sizeof(user_entity_base_block))/(6*sizeof(unsigned long))
typedef  struct user_port_addr_block
{
    int addr_type;
    union {
        unsigned long addr;
        char addr_string[64];
    }port_logic_addr;

    union {
        unsigned long phy_addr;
        char phy_addr_string[64];
    }port_phy_addr;

}user_port_addr_block;

typedef struct user_entity_base_block
{
    char state;
    int user_name_len;
    int group_name_len;
    int port_num;
    int relate_wireway_num;
    int relate_point_num;
    unsigned long name_id;
    unsigned long block_id;
    unsigned long user_token;
    int used_port_num;
    int used_wireway_num;
    int used_point_num;
}user_entity_base_block;

typedef struct user_port_base_block
{
    int port_len;
    char port_name[32];
    int port_index;
    int port_type;
    int port_state;
    int addr_num;
    user_port_addr_block addr_block[0];
}user_port_block;

typedef struct create_wire_desc
{
    int index;
    unsigned long wireway_name_id;
}create_wire_block;

typedef struct attach_point_desc
{
    unsigned long wireway_name_id;
    int port_index;
    int index;
}attach_point_block;

typedef struct user_entity_content_block
{
    int obj_num;
    char content[USER_BLOCK_CONTENT_LEN];
}user_entity_content_block;


typedef struct user_entity_root_block
{
    user_entity_base_block base_info;

    unsigned long port_storage_id[USER_BLOCK_MAX_STORAGE_ARRAY];
    unsigned long create_wire_storage_id[USER_BLOCK_MAX_STORAGE_ARRAY];
    unsigned long attach_point_storage_id[USER_BLOCK_MAX_STORAGE_ARRAY];    
}user_entity_root_block;

typedef struct entity_req
{
    int msg_type;
    char *name;
    char *group_name;
    unsigned long reg_token;
    int msg_size;
    char content[0];
}entity_req;



unsigned long get_user_entity_key(void *data);
unsigned long get_user_entity_data(void *data);
unsigned long alloc_user_bptree_block();
unsigned long alloc_user_entity_block();




#endif

