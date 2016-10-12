#include <linux/slab.h>
#include "wireway_node.h"

#define CACHE_PAGE_NUM (PAGE_SIZE/sizeof(id_node))
#define CACHE_PAGE_MASK (CACHE_PAGE_NUM -1)



struct list_head collector_list = 
{
    .prev = &collector_list,
    .next = &collector_list
};

void * call_for_create_wireway(int spider_addr,char *wireway_name)
{
    



}

void * call_for_attach_wireway(int spider_addr,char *wireway_name)
{




}

wireway_collector*  lookup_collector(char *name)
{
    wireway_collector *ctmp = NULL;

    list_for_each_entry(ctmp,&collector_list,node_list)
    {
        if(0 == strcmp(ctmp->name,name))
        {
            return ctmp;
        }
    }
    return NULL;
}


int create_collector(char *name)
{
    wireway_collector *ctmp ,*node,*node_pre ;

    if(strlen(name) >= 32)
    {
        return -1;
    }

    
    node_pre = &collector_list;

    list_for_each_entry(ctmp,&collector_list,node_list)
    {
        if(strcmp(ctmp->name,name) == 0)
        {
            return -1;
        
        }else if(0 > strcmp(ctmp->name,name)) {

            node_pre = ctmp;
    
        }else{
            break;

        }     
    }
    
    node = kmalloc(sizeof(wireway_collector),GFP_KERNEL);
    if(node)
    {
        memset(node,0,sizeof(wireway_collector));
        strcpy(node->name,name);
        INIT_LIST_HEAD(&node->node_list);
        node->rcv_queue = lfrwq_init(1024,128,6);
        if(!node->rcv_queue)
        {
            kfree(node);
            return -1;
        }
        list_add(&node->node_list,&ctmp->node_list); 
         
    }

    return 0; 
}
int get_size_order(unsigned long long  size)
{
    int order = 0;
    while(size)
    {
        if(size %2 != 0)
        {
            return -1;
        }
        size = size >>1;
        order++;
    } 
    return order;
}
id_cache_control* create_id_cache(void)
{
    id_cache_control *cache = kmalloc(sizeof(id_cache_control),GFP_KERNEL);
    if(cache)
    {
        cache->high = 1;
        cache->size = PAGE_SIZE/sizeof(id_node);
        cache->mask = cache->size -1;
        cache->shift = get_size_order(cache->size); 
        cache->page_cache = get_zeroed_page(GFP_KERNEL);
        if(cache->page_cache)
            return cache;
    }
    return NULL;
}

void*  cache_id_lookup(id_cache_control *cache, unsigned long long id)
{
    unsigned int shift,mask,high;
    unsigned long long id_tmp;
    unsigned long long prefix;

    if(cache)
    {
        shift = cache->shift;
        mask =  cache->mask;
        high = cache->high;
        id_node *node_page_start= cache->page_cache;
        prefix = cache->prefix;
        
        id_tmp = id >> (high*shift);
        if(id_tmp != prefix)
        {
            return NULL;   
        }        

        
        while(high)
        {
            id_tmp = id >> ((high -1)*shift);
            id_tmp = id_tmp&mask;
            id_node node = node_page_start[id_tmp];
            if(1 == high)
            {
                return node.node_ptr; 
            }
            else
            {
                node_page_start = node.node_ptr;
                if(!node_page_start)
                {
                    return NULL;
                }
            }
         
            high--;
        }

    }   


    return NULL;
}


int cache_id_insert(id_cache_control * cache,unsigned long long id,void* item)
{
    unsigned int shift,mask,high;
    unsigned long long id_tmp;
    unsigned long long prefix;
    if(cache)
    {
        shift = cache->shift;
        mask = cache->mask;
        high = cache->high; 
        id_node *node_page_start= cache->page_cache;
        prefix = cache->prefix;
        id_tmp = id >> (high*shift);
        if(id_tmp != prefix)
        {
            cache->page_cache = get_zeroed_page(GFP_KERNEL);
            cache->prefix = id_tmp >> shift;
            cache->high++;
            id_node *node_page_new = cache->page_cache;
            node_page_new[prefix].page_ptr = node_page_start;
            node_page_new[id_tmp].page_ptr = get_zeroed_page(GFP_KERNEL);

            node_page_start = node_page_new[id_tmp].page_ptr;                       
        }

        while(high)
        {
            id_tmp = id >> ((high -1)*shift);
            id_tmp = id_tmp&mask;
            id_node node = node_page_start[id_tmp];
            if(1 == high)
            {
                if(node.node_ptr != 0)
                {
                        //chong fu
                        return -1;
                }
                node.node_ptr = item;
            }
            else
            {
                node_page_start = node.node_ptr;
                if(!node_page_start)
                {
                    node_page_start = node.node_ptr = get_zeroed_page(GFP_KERNEL);
                }
            }
            
            high--;
    
        } 

    }
    return -1;            
}
