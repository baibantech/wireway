#include <linux/list.h>

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
    char *name;
    struct list_head node_list;

}wireway_collector;



