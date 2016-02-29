#include "user_entity.h"
#include "bptree.h"

typedef struct reg_entity_req
{
    int msg_type;
    char *name;
    char *group_name;
    unsigned long reg_token;
    int msg_size;
    char content[0];    
}reg_entity_req;


tree_root user_entity_control
{
    .save = 1 ,
    .key_cmp = strcmp,
    .get_key_id_func = get_user_entity_key,
    .get_data_id_func = get_user_entity_data,
    .node_root = NULL,
};




unsigned long prepare_reg_entity(reg_entity_req *req)
{
    char key_name[64] = {0};
    int len = 0;
    if(NULL == req || 0 == strlen(req->name)|| 0== strlen(group_name))
    {
        return -1;
    }
    if(strlen(req->name) + strlen(group_name) > 64-2)
    {
        return -1;
    }
    if(group_name[strlen(group_name)-1]!= `#')
    {
        sprintf(key_name,"%s#%s",req->group_name,req->name);
        if(is_user_entity_exist(key_name))
        {
            return -1;
        }
        else
        {
            

        }
    }
    else
    {
        return -1;
    }   
    

    



}


user_entity_desc *reg_entity(reg_entity_req *req)
{
    



}


