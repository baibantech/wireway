#include "wireway.h"
int init_wireway(wireway *w,char *name);
point* get_unused_peer(wireway *w);
void insert_wireway_tree(wireway *w);
void insert_bridge_point(bridge_point *b,wireway *src);
void update_wireway_fib(point *p,wireway *srcw);
void print_wireway();
wireway* lookup_wireway(char *name);
void destroy_wireway(wireway *w);
void assgin_bridge_location(bridge_point *b);
int is_wireway_exist(char *name);
wireway *alloc_wireway_inst();
void wireway_tree_restore();
wireway *find_wireway_restore_list(char *name);
wireway *get_wireway_in_mem(char *name);
unsigned long get_bptree_dataid(void *data);

unsigned long get_bptree_keyid(void *data);

unsigned long alloc_wireway_bptree_block();

unsigned long alloc_wireway_block();
