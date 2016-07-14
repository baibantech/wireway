#include "user_entity.h"
#include "bptree.h"
#include "wireway.h"

char *serial_entity_req(entity_req *req,char **msg)
{
    int name_len,group_name_len;
    int mem_len ;
    char *serial_mem = NULL;
    char *mem_tmp = NULL;
    if(!req || !msg)
    {
        return -1;
    }
    
    name_len = strlen(req->name);
    group_name_len = strlen(req->group_name);
    if(0 == name_len || 0 == group_name_len)
    {
        return -1;
    } 
    mem_len = name_len + group_name_len + req->msg_size + 4*sizeof(int) + sizeof(unsigned long);

    serial_mem = malloc(mem_len);
    if(NULL == serial_mem)
    {
        return -1;
    }    
    
    mem_tmp = serial_mem;
    
    *(int*)(mem_tmp) = req->msg_type;
    mem_tmp += sizeof(int);

    *(int*)(mem_tmp) = name_len;
    mem_tmp += sizeof(int);

    strcpy(mem_tmp,req->name);
    mem_tmp += name_len;

    *(int*)mem_tmp = group_name_len;
    mem_tmp += sizeof(int);

    strcpy(mem_tmp ,req->group_name);
    mem_tmp += group_name_len;

    *(unsigned long*)mem_tmp = req->reg_token;
    mem_tmp += sizeof(unsigned long);

    *(int*)mem_tmp = req->msg_size;
    mem_tmp +=  sizeof(int);
    
    memcpy(mem_tmp,req->content,req->msg_size);
    *msg = serial_mem;
    return mem_len;
}


entity_req *construct_req(int type,char *name,char *group_name,int size,char *content)
{
    int name_len,group_name_len;
    int msg_len = sizeof(entity_req) + size + 1;
    if(NULL == name || NULL == group_name || NULL == content)
    {
        return NULL;
    } 
         
    name_len = strlen(name);
    group_name_len = strlen(group_name); 
    
    if( 0 == name_len || 0 ==  group_name_len )
    {
        return NULL;
    }

    entity_req *req = malloc(msg_len);
    
    if(NULL == req)
    {
        return NULL;
    }
    memset(req,0,sizeof(entity_req));
    
    req->msg_type = type;
    req->name = malloc(name_len +1);
    if(NULL == req->name)
    {
        free(req);
        return NULL;
    }
    strcpy(req->name,name);
    req->group_name = malloc(group_name +1);
    
    if(NULL == req->group_name)
    {
        free(req->name);
        free(req);
        return NULL;
    }

    strcpy(req->group_name,group_name);

    req->msg_size = size;

    memcpy(&req->content[0] ,content,size);

    return req;
}

unsigned long prepare_reg_entity(entity_req *req)
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
    free(key_name);
    return dsc->user_token;
}

int reg_entity(entity_req *req)
{
    int len = 0;
    int name_len , group_name_len = 0;
    char *key_name = NULL;
    user_entity_desc *dsc = NULL;
    int port_num = 0;
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
        dsc = lookup_user_entity(key_name);
        if(!dsc)
        {
            return -1;
        }
        
        if(req->reg_token != dsc->user_token)
        {
            return -1;
        }

        /*parse port content*/
        port_num = load_port_content(req->content,dsc);
        if(-1 != port_num)
        {
            dsc->port_num = port_num;
            return 0;
        }

    }
    return -1;    
}
