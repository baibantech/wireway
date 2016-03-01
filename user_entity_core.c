#include "user_entity.h"
#include "bptree.h"

int storage_zone_usr_bptree_id ;



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



}

int init_user_entity(user_entity_desc *desc)
{

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
