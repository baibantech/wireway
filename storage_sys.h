#ifndef __storage_sys__
#define __storage_sys__
#include <stdio.h>
#include "list.h"
enum zone_type
{
    zone_block_type,
    zone_free_type,
    zone_type_max, 
};

typedef struct storage_block_file_tag
{
    int block_free_count;
    int root_id;
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


#endif
