#include "user_entity.h"
#include "bptree.h"

typedef struct user_entity_base_block
{
    char state;
    int user_name_len;
    int group_name_len;
    int port_num;
    unsigned long name_id;
    unsigned long user_token;
}user_entity_base_block;

typedef struct user_port_base_block
{
    char port_name[32];
    int port_index;
    int port_type;
    int port_state;
    int addr_num;
}user_port_base_block;

typedef  struct user_port_addr_block
{
    union {
        unsigned long addr;
        char addr_string[64];
    }port_logic_addr;

    union {
        unsigned long phy_addr;
        char phy_addr_string[64];
    }port_phy_addr;

}user_port_addr_block;

int storage_zone_usr_bptree_id = -1 ;

tree_root user_entity_control
{
    .save = 1 ,
    .key_cmp = strcmp,
    .get_key_id_func = get_user_entity_key,
    .get_data_id_func = get_user_entity_data,
    .alloc_node_func = alloc_user_entity_block;
    .node_root = NULL,
};

int user_entity_storage_zone_reg()
{
    storage_zone_usr_bptree_id = reg_storage_zone("usr_bptree",zone_block_type,sizeof(user_entity_block)+4,1000)
    if(-1 == storage_zone_usr_bptree_id)
    {
        return -1;
    }
    return 0;
}

unsigned long get_user_entity_key(void *data)
{
    user_entity_desc *desc = (user_entity_desc*)data;
    return desc->name_id;
}

unsigned long get_user_entity_data(void *data)
{
    user_entity_desc *desc = (user_entity_desc*)data;
    return desc->user_block_id;
}

unsigned long alloc_user_entity_block()
{
    return alloc_block_from_zone(storage_zone_usr_bptree_id);
}

user_entity_desc *alloc_user_entity_inst()
{
    user_entity_desc *dsc = NULL;
    dsc = (user_entity_desc*)malloc(sizeof(user_entity_desc));
    if(dsc)
    {
        memset(dsc,0,sizeof(user_entity_desc));
        dsc->user_block_id = alloc_user_entity_block();
        if(-1 == dsc->user_block_id)
        {
            free(dsc);
            return NULL;
        }
    }
    return NULL;
}
void save_user_entity(user_entity_desc *desc)
{



}

int init_user_entity(user_entity_desc *desc,reg_entity_req *req)
{
    int len;
    char *name = NULL;
    char *key_name = NULL;
    name = malloc(strlen(req->name)+1);
    if(!name){
        return -1; 
    }
    strcpy(name,req->name);
    desc->name = name;
    
    name = malloc(strlen(req->group_name)+1);
    if(!name){
        free(desc->name);
        return -1;
    }
    key_name = malloc(strlen(req->name)+strlen(req->group_name) +2);
    if(!key_name){
        free(desc->name);
        free(desc->group_name);
        return -1;
    }     
    sprintf(key_name,"%s#%s",req->name,req->group_name);
    strcpy(name,req->group_name);
    desc->group_name = name;
    desc->state = user_entity_pre_reg;     
    desc->user_token = assign_user_entity_token(desc);
    desc->name_id = save_name(key_name);
    if(-1 == desc->name_id){
        free(key_name);
        free(desc->group_name);
        free(desc->name);
        renturn -1;
    }

    save_user_entity(desc);
}
void user_entity_tree_restore()
{
    node *usr = restore_bptree_root(storage_zone_usr_bptree_id);
    if(usr)
    {
        user_entity_control.node_root = usr;
        usr->tree_root = &user_entity_control;
    }
}

void insert_user_entity_tree(user_entity_desc *usr_desc)
{
    if(NULL == user_entity_control.node_root)
    {
        user_entity_control.node_root = make_new_tree(&user_entity_control,usr_desc->name,usr_desc);
    }
    else
    {
        user_entity_control.node_root = insert(user_entity_control.node_root,usr_desc->name,usr_desc);
    }
}

user_entity_desc *lookup_user_entity(char *name)
{
    if(NULL != name)
    {
        return (user_entity_desc*)find(user_entity_control.node_root,name);
    }
    return NULL;
}
int is_user_entity_exist(char *name)
{
    if(NULL != name)
    {
        return is_key_exist(user_entity_control.node_root,name);
    }
}
