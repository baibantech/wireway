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

unsigned long prepare_reg_entity(reg_entity_req *req)
{
    int len = 0;
    char *key_name = NULL;
    user_entity_desc *dsc = NULL;
    if(NULL == req || 0 == strlen(req->name)|| 0== strlen(group_name))
    {
        return -1;
    }
    if(group_name[strlen(group_name)-1]!= `#')
    {
        key_name = malloc(strlen(group_name)+strlen(name)+1);
        if(NULL == key_name){
            return -1;
        } 
        sprintf(key_name,"%s#%s",req->group_name,req->name);
        if(is_user_entity_exist(key_name))
        {
            free(key_name);
            return -1;
        }
        else
        {
            dsc = alloc_user_entity_inst();
            if(NULL == dsc){
                free(key_name);
                return -1;   
            }
            
            if(-1 == init_user_entity(dsc,req)){
                free(key_name);
                return -1;
            }
            
            insert_user_entity_tree(dsc);               
        }
    }
    else
    {
        return -1;
    }   

    return dsc->user_token;
}


user_entity_desc *reg_entity(reg_entity_req *req)
{
    



}


