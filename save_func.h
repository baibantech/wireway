#include "wireway.h"
unsigned long save_name(char *key);
unsigned long alloc_bptree_block();
unsigned long alloc_wireway_block();
unsigned long get_bptree_rootid();
unsigned long get_bptree_dataid(void *data);
unsigned long get_bptree_keyid(void *data);
void save_data(unsigned long id,void *data,int len);
void *read_data(unsigned long id);
void save_wireway_block(wireway_block *block);

