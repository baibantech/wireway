#include <linux/list.h>
#include "lf_rwq.h"

typedef struct wireway_peer_node 
{
    

}wireway_peer_node;

typedef struct wireway_node 
{
    char *name;
    unsigned long global_id;
    int  local_id;
    struct wireway_collector *collector;         
    struct list_head list;
    struct hlist_head hlist;    
}wireway_node;

typedef struct wireway_collector
{
    char  name[32];
    unsigned long  collector_id;
    lfrwq_t *rcv_queue; 
    struct list_head node_list;

}wireway_collector;

typedef struct id_node
{
    union
    {
        void* node_ptr;
        void* page_ptr;
    };

}id_node;


typedef struct id_cache_control
{
    unsigned int high;
    unsigned int size;
    unsigned int shift;
    unsigned int mask;
    unsigned long prefix;
    void *page_cache;
}id_cache_control;

wireway_collector*  lookup_collector(char *name);
unsigned long  create_collector(char *name);
void *cache_id_lookup(id_cache_control *cache,unsigned long id);
int cache_id_insert(id_cache_control *cache,unsigned long id,void *item);
id_cache_control *create_id_cache(void);
void collector_cache_init(void);

extern id_cache_control *collector_cache ;
