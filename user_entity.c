#include "user_entity.h"
#include "bptree.h"
#include "wireway.h"

unsigned long prepare_reg_entity(reg_entity_req *req)
{
    int len = 0;
    int name_len , group_name_len = 0;
    char *key_name = NULL;
    user_entity_desc *dsc = NULL;
    if(NULL == req || 0 == strlen(req->name)|| 0== strlen(req->group_name))
    {
        return -1;
    }
    name_len = strlen(req->name);
    group_name_len = strlen(req->group_name);    

    if(req->group_name[group_name_len-1]!= '#')
    {
        key_name = malloc(name_len + group_name_len+1);
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


