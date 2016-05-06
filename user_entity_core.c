#include "wireway.h"
#include "user_entity.h"
#include "bptree.h"

int storage_zone_usr_bptree_id = -1 ;

tree_root user_entity_control
{
    .save = 1 ,
    .key_cmp = strcmp,
    .get_key_id_func = get_user_entity_key,
    .get_data_id_func = get_user_entity_data,
    .alloc_node_func = alloc_user_bptree_block;
    .node_root = NULL,
};

int user_entity_storage_zone_reg()
{
    storage_zone_usr_bptree_id = reg_storage_zone("usr_bptree",zone_block_type,sizeof(user_entity_block)+4,1000);
    if(-1 == storage_zone_usr_bptree_id)
    {
        return -1;
    }
    storage_zone_usr_entity_id = reg_storage_zone("usr_entity",zone_block_type,sizeof(user_entity_content_block)+4,1000);
    if(-1 == storage_zone_usr_entity_id)
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

unsigned long alloc_user_bptree_block()
{
    return alloc_block_from_zone(storage_zone_usr_bptree_id);
}

unsigned long alloc_user_entity_block()
{
    return alloc_block_from_zone(storage_zone_usr_entity_id);
}

user_entity_desc *alloc_user_entity_inst()
{
    user_entity_desc *dsc = NULL;
    user_entity_block *block = NULL;
    dsc = (user_entity_desc*)malloc(sizeof(user_entity_desc));
    if(dsc)
    {
        memset(dsc,0,sizeof(user_entity_desc));
        block = malloc(sizeof(user_entity_root_block));
        if(NULL == block)
        {
            free(dsc);
            return NULL;
        }
        memset(block ,0,sizeof(user_entity_root_block)); 

        dsc->root_block_id = alloc_user_entity_block();
        if(-1 == dsc->root_block_id)
        {
            free(dsc);
            free(block);
            return NULL;
        }
        
        save_data(dsc->root_block_id,block,sizeof(user_entity_root_block));
        desc->block = block;
    }
    return NULL;
}
int save_user_entity(user_entity_desc *desc)
{
    /*save base information*/
    user_entity_base_block *base_info = NULL;
    user_logic_port *port;
    user_port_addr *addr;
    user_port_block *port_block,*port_head = NULL;
    create_wire_block *create_wire = NULL;
    attach_point_block *attach_point = NULL;
    wireway *w = NULL;    
    int base_info_len ,port_info_len,create_wire_info_len,attach_point_len;
    unsigned long *storage_array = NULL;    
    user_entity_root_block *root_block = desc->block;  
    
    unsigned long root_storage_id = curr_storage_id = -1;

    struct list_head *pos,*pos1;
    int len = 0;
    int i,j,k;
    
    int port_info_offset;

    /*collect base info*/
    root_block->base_info->state = desc->state;
    root_block->base_info->user_name_len = strlen(desc->name);
    root_block->base_info->group_name_len = strlen(desc->group_name);
    root_block->base_info->port_num = desc->port_num;
    root_block->base_info->relate_wireway_num = desc->relate_wireway_num;
    root_block->base_info->relate_point_num = desc->relate_point_num;
    root_block->base_info->name_id = desc->name_id;
    root_block->base_indo->block_id = desc->user_block_id;
    root_block->base_info->user_token = desc->user_token;

    root_storage_id = desc->root_block_id;

    save_data(root_storage_id,root_block,sizeof(user_entity_root_block)); 
    
    /*collect port infomation*/
    k = 0;
    i = 0;
    if(desc->port_num){
        port_info_offset = sizeof(int);//objnum offset
        list_for_each(pos,&desc->port_list)
        {
            port  = list_entry(pos,user_logic_port,list);
            len = sizeof(user_port_block)+ port->addr_num*sizeof(user_port_addr_block);
            if(len >= USER_BLOCK_CONTENT_LEN - sizeof(int)){
                printf("the port block len is too long\r\n");
                return -1;
            }
            port_head = malloc(len);
            if(NULL = port_head) {
                goto free_port;
            }
            port_head->port_len = len; 
            port_head->port_index = port->port_index;
            port_head->port_type = port->port_type;
            port_head->port_state = port->port_state;
            port_head->addr_num = port->port_num;
            strcpy(port_head->port_name,port->port_name);
            j = 0;        
            list_for_each(pos1,&port->addr_list)
            {
                addr = list_entry(pos1,user_port_addr,list);
                port_head->addr_block[j].type = addr->type; 
                port_head->addr_block[j].port_logic_addr = addr->port_logc_addr;
                port_head->addr_block[j].port_phy_addr = addr->port_phy_addr;
                j++;
            }
       
            if(port_info_offset + len < USER_BLOCK_CONTENT_LEN){
                /*save data in new block*/
                if(curr_storage_id == -1)
                {
                    curr_storage_id = alloc_user_entity_block();
                    if(-1 == curr_storage_id)
                    {
                        return -1;
                    }
                }
                i++;    
            }
            else
            {
                /*exchage old block*/
                if(-1 != root_block->port_storage_id[k])
                {
                    free_data(root_block->port_storage_id[k]);
                }
                root_block->port_storage_id[k] = curr_storage_id;
                k++;
                save_data(curr_storage_id,0,&i,sizeof(i));
                i = 1;
                curr_storage_id = alloc_user_entity_block();
                if(-1 == curr_storage_id)
                {
                    return -1;
                }
                port_info_offset = sizeof(int);
            }
            save_data_offscet(curr_storage_id,port_info_offset ,port_head,len);
            port_info_offset += len; 
        }
    } 
    /*collect create wireway info*/
    curr_storage_id = -1;
    i  = 0;
    k = 0;
    create_wire_info_len = desc->relate_wireway_num*sizeof(create_wire_block);
    block_count = USER_BLOCK_CONTENT_LEN/sizeof(create_wire_block);
    create_wire_offset_index = 0;
    if(create_wire_info_len){
        create_wire = malloc(create_wire_info_len);
        if(NULL == create_wire)
        {
            goto free_port;
        }

        list_for_each(pos,&desc->create_wire_list)
        {
            w = list_entry(pos,wireway,usr_list);
            create_wire[i]->wireway_name_id = w->block->name_id;
            create_wire[i]->index = i; 
            i++;
            if(0 == (i% block_count))
            {
                curr_storage_id = alloc_user_entity_block();
                if(curr_storage_id == -1)
                {
                    return -1;
                }
                save_data(curr_storage_id,sizeof(int),&create_wire[curr_wire_offset_index],sizeof(create_wire_block)*(i-curr_wire_offset_index);
                root_block->create_wire_storage_id[k++] = curr_storage_id;
                curr_storage_id = -1;
                curr_wire_offset_index = i - 1;
            }
        }
        if(curr_wire_offset_index < i -1)
        {
            curr_storage_id = alloc_user_entity_block();
            if(curr_storage_id == -1)
            {
                return -1;
            }
            save_data(curr_storage_id,sizeof(int),&create_wire[curr_wire_offset_index],sizeof(create_wire_block)*(i-curr_wire_offset_index);
            root_block->create_wire_storage_id[k++] = curr_storage_id;
        }
    }
    
    /*collect attach point info*/
    curr_storage_id = -1;
    i  = 0;
    k = 0;
    attach_point_info_len = sizeof(attach_point_block)*desc->relate_point_num;
    block_count = USER_BLOCK_CONTENT_LEN/sizeof(attach_point_block);
    attach_point_offset_index = 0;
    if(attach_point_info_len){    
        attach_point = malloc(sizeof(attach_point_block)*desc->relate_point_num);
        if(NULL == attach_point)
        {
            goto free_create_wire;
        }

        i = 0;
        list_for_each(pos,&desc->attach_point)
        {
            point *p = list_entry(pos,point,usr);
            attach_point[i]->index = i;
            attach_point[i]->wireway_name_id = p->wire->block->name_id;
            attach_point[i]->port_index = p->index; 
            i++;
            if(0 == (i% block_count))
            {
                curr_storage_id = alloc_user_entity_block();
                if(curr_storage_id == -1)
                {
                    return -1;
                }
                save_data(curr_storage_id,sizeof(int),&attach_point[attach_point_offset_index],sizeof(attach_point_block)*(i-curr_wire_offset_index);
                root_block->attach_point_storage_id[k++] = curr_storage_id;
                curr_storage_id = -1;
                attach_point_offset_index = i - 1;
            }
            
        }

        if(0 == (i% block_count))
        {
            curr_storage_id = alloc_user_entity_block();
            if(curr_storage_id == -1)
            {
                return -1;
            }
            save_data(curr_storage_id,sizeof(int),&attach_point[attach_point_offset_index],sizeof(attach_point_block)*(i-curr_wire_offset_index);
            root_block->attach_point_storage_id[k++] = curr_storage_id;
        }


    
    }

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
    
    desc->port_num = 0;
    INIT_LIST_HEAD(&desc->port_list);

    desc->relate_wireway_num = 0;
    INIT_LIST_HEAD(&desc->create_wire_list);

    desc->relate_point_num = 0;
    INIT_LIST_HEAD(&desc->attach_point);

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
