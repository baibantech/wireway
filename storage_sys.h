#ifndef __storage_sys__
#define __storage_sys__
#include <stdio.h>
#include "list.h"
#include "wireway.h"
#include "bptree.h"

#define MAX_OPEN_MEDIA_FILE 100
enum zone_type
{
    zone_block_type,
    zone_free_type,
    zone_type_max, 
};

typedef struct storage_block_file_tag
{
    int block_free_count;
    long bitmap[0];
}storage_block_file;

typedef struct storage_free_file_tag
{
    int free_size;
    int free_area_offset;
    int used_area_offset; 
}storage_free_file;
                            
typedef struct storage_media_tag
{
    int file_index;
    FILE *file_handle;
    struct storage_media_tag *next;
    union
    {
        storage_block_file block_file;
        storage_free_file  free_file;
    }file;
}storage_media;

typedef struct storage_zone_tag
{
    char zone_name[32];
    int  zone_id;
    int  zone_type;
    int  block_size;
    int  block_count;
    int  media_count;
    unsigned long root_id;
    storage_media *media_list;
    storage_media *cur_media;
    struct storage_zone *next;    
}storage_zone;

typedef struct storage_zone_head_tag
{
    int zone_num;
    storage_zone *zone_list;
}storage_zone_head;

typedef struct storage_buddy_sys_tag
{
    char buddy_sys_name[32];
    int order_num;
    int order_min;
    int order_max;
    int order_array[8];
    storage_zone block_zone[8];
    storage_zone free_zone; 
}storage_buddy_sys;

typedef struct storage_buddy_head_tag
{
    int sys_num;
    storage_buddy_sys *sys_list;
}storage_buddy_head;

typedef struct file_handle_item
{
    


}file_handle_item;


storage_zone *alloc_storage_zone(char *zone_name,int zone_type,int block_size,int per_media_count);

int alloc_storage_zone_id();

unsigned long alloc_block_from_zone(int zone_id);

void free_storage_zone(storage_zone *zone);

void free_key_block(unsigned long id);

void free_data_block(unsigned long id);

FILE *find_file_stream(storage_zone *zone,int file_index);

unsigned long get_bptree_rootid();

storage_zone *get_zone(int zone_id);

storage_zone *restore_storage_zone(char *zone_name);

int reg_storage_zone(char *zone_name,int zone_type,int block_size,int per_media_count);

int reg_storage_buddy_system(char *buddy_name,char order_array[],int order_num);

void *read_data(unsigned long storage_id);


void save_data(unsigned long storage_id,void *data,int len);

unsigned long save_name(char *key);

void save_bptree_node(node_block *block);

void save_bptree_root_node(node_block *block);

void set_bptree_root_id(unsigned long root_id,unsigned long zone_id);

void save_wireway_block(wireway_block *block);

#endif
