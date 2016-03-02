#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bptree.h"
#include "bitmap.h"
#include "wireway.h"
#include "storage_sys.h"


int storage_zone_wireway_bptree_id = -1;
int storage_zone_key_id = -1;
int storage_zone_wireway_id = -1;

storage_zone_head *global_zone_head = NULL;
storage_buddy_head *buddy_head = NULL;
int global_zone_id = 0;
int open_media_file_count = 0;

void free_storage_zone(storage_zone *zone)
{
    storage_media *media,*media_tmp;
    if(NULL == zone)
    {
        return ;
    }

    media = zone->media_list;
    while(media)
    {
        media_tmp = media;
        media = media->next;
        free(media_tmp);
    }    

    free(zone);
    return;
}

storage_zone *restore_storage_zone(char *zone_name)/*zone_name file  exist*/
{
    FILE *file;
    int fd;
    storage_zone *zone;
    int i;
    storage_media *media ,*media_tmp;
    char tmp_file_name[32+10];
    file = fopen(zone_name,"rb+");
    if(file){
        zone = malloc(sizeof(storage_zone));
        if(zone){
            memset(zone,0,sizeof(storage_zone));
            fread(zone,sizeof(storage_zone),1,file);
            zone->media_list = NULL;
            zone->cur_media = NULL;
            zone->next = NULL;
            for(i = 0 ; i < zone->media_count ; i++){
                switch(zone->zone_type)
                {
                    case zone_block_type:
                    {                   
                        int len = sizeof(storage_media)+(zone->block_count/sizeof(long) +1)*sizeof(long);
                        media = malloc(len);
                        if(media){
                            if(0 == i){
                                fseek(file,sizeof(storage_zone),SEEK_SET);
                                fread(media,len,1,file);
                                media->file_handle = file;
                                media->next = NULL;
                                media_tmp = media;
                                zone->media_list = media;
                                zone->cur_media = media;                               
                            } 
                            else{
                                sprintf(tmp_file_name,"%s#%d",zone_name,i);
                                file = fopen(tmp_file_name,"rb+");
                                if(file){
                                    fread(media,len,1,file);
                                    media->file_handle = NULL;
                                    media->next = NULL;
                                    media_tmp->next = media;
                                    fclose(file);
                                }
                            }
                        }                      
                        break;
                    }
                    case zone_free_type:
                    {
                        printf("zone type un support\r\n");
                        break;
                    }
                    default:
                        printf("zone type error\r\n");
                        break;
                }
            }
             
        }
        else
        {
            goto free_file;
        }
        return zone;
    }
    return NULL;
    
    free_file:
    if(file) fclose(file);
    
    free_zone:
    free_storage_zone(zone);
    return NULL;
}
int alloc_storage_zone_id()
{
    return global_zone_id++; 
}
storage_zone *alloc_storage_zone(char *zone_name,int zone_type,int block_size,int per_media_count)
{
    storage_zone *zone_tmp;
    storage_media *media,*media_tmp;
    int len;
    FILE *f;
    zone_tmp = malloc(sizeof(storage_zone));
    if(zone_tmp)
    {
        strcpy(zone_tmp->zone_name,zone_name);
        zone_tmp->zone_type = zone_type;
        zone_tmp->block_size = block_size;
        zone_tmp->media_count = 1;
        zone_tmp->block_count = per_media_count;
        zone_tmp->root_id = -1;
        if(zone_type == zone_block_type)
        {
            len = sizeof(storage_media)+(zone_tmp->block_count/sizeof(long) +1)*sizeof(long);
            media = malloc(len);
            if(media)
            {
                memset(media,0,len);
                media->file_index = 0;
                media->file.block_file.block_free_count = per_media_count;
                f = fopen(zone_name,"w+");
                if(f)
                {
                    int fd = fileno(f);
                    posix_fallocate(fd,0,len);
                    zone_tmp->zone_id = alloc_storage_zone_id();
                    fwrite(zone_tmp,sizeof(storage_zone),1,f);
                    fseek(f,sizeof(storage_zone),SEEK_SET);
                    fwrite(media,len,1,f);
                    if(open_media_file_count < MAX_OPEN_MEDIA_FILE)
                    {
                        media->file_handle = f;
                        open_media_file_count++;
                    }
                    else
                    {
                        media->file_handle = NULL;
                        fclose(f);
                    }
                    zone_tmp->media_list = media;
                    zone_tmp->cur_media = media;
                    return zone_tmp;
                }
            }
            goto free_zone;
        }
        else if(zone_type == zone_free_type)
        {
            printf("zone type un supported\r\n");
            goto free_zone;
        }
        else
        {
            printf("zone type err\r\n");
            goto free_zone;
        }
    }
    free_zone:
    if(zone_tmp)
    {
        free_storage_zone(zone_tmp);
    }    
    return NULL;
}


int reg_storage_zone(char *zone_name,int zone_type,int block_size,int per_media_count)
{
    storage_zone *zone_tmp , *zone_pre ;
    int len = 0;
    storage_media *media;
    FILE *f = NULL;
    if(strlen(zone_name) >=32)
    {
        return -1;
    }

    if(zone_type >=zone_type_max)
    {
        return -1;
    }

    if(NULL == global_zone_head)
    {
        global_zone_head = malloc(sizeof(storage_zone_head));
        if(NULL == global_zone_head) return -1;
        memset(global_zone_head,0,sizeof(storage_zone_head));
    }
    zone_pre = zone_tmp = global_zone_head->zone_list;
    while(zone_tmp)
    {
        if(0 == strcmp(zone_tmp->zone_name,zone_name))
        {
            return -1;
        }
        zone_pre = zone_tmp;
        zone_tmp = zone_tmp->next;
    }
    
    if(access(zone_name,0)== 0)
    {
        zone_tmp = restore_storage_zone(zone_name);
    } 
    else
    {
        zone_tmp = alloc_storage_zone(zone_name,zone_type,block_size,per_media_count);
    }
    
    if(NULL == zone_tmp)
    {
        goto free_zone_head;
    }

    if(NULL == zone_pre)
    {
        global_zone_head->zone_list = zone_tmp;
    }
    else
    {
        zone_pre->next = zone_tmp;
    }
    global_zone_head->zone_num++;
    return zone_tmp->zone_id;

free_zone_head:
    free(global_zone_head);
    global_zone_head = NULL;
    return -1;
}

int reg_storage_buddy_system(char *buddy_name,char order_array[] ,int order_num)
{





    return -1;
}
void *alloc_space_from_buddy_sys(int sys_id,int len)
{


}
storage_zone *get_zone(int zone_id)
{
    storage_zone *zone = NULL;
    if(NULL == global_zone_head)
    {
        return NULL;
    }
    zone = global_zone_head->zone_list;
    while(zone)
    {
        if(zone->zone_id == zone_id)
        {
            return zone;
        }
        zone = zone->next;
    }
    return NULL;
}

unsigned long alloc_block_from_zone(int zone_id)
{
    storage_zone *zone = get_zone(zone_id);
    storage_media *media = NULL;
    unsigned long  block_index = 0;
    FILE *file;
    int len = 0;
    if(zone)
    {
        media = zone->cur_media;
        if(media)
        {
            switch(zone->zone_type)
            {
                case zone_block_type:
                {
                    unsigned long zone_id = zone->zone_id;
                    block_index = find_next_zero_bit(media->file.block_file.bitmap,zone->block_count,0);
                    len = sizeof(storage_media)+(zone->block_count/sizeof(long) +1)*sizeof(long);
                    set_bit(block_index,media->file.block_file.bitmap);
                    media->file.block_file.block_free_count++;
                    file = media->file_handle;
                    if(0 == media->file_index)
                    {
                        fseek(file,sizeof(storage_zone),SEEK_SET);
                    }
                    else
                    {
                        fseek(file,0,SEEK_SET);
                    }
                    fwrite(media,len,1,file);
                    return (block_index +media->file_index *zone->block_count)|(zone_id << (BITS_PER_LONG- BITS_PER_BYTE));            
                }
                
                case zone_free_type:
                default:
                printf("zone type err\r\n");
                return -1;
             }
        }
    }

    return -1;
}

FILE *find_file_stream(storage_zone *zone,int file_index)
{
    storage_media *media = zone->media_list;
    while(media)
    {
        if(media->file_index == file_index)
        {
            return media->file_handle;
        }
    }
    return NULL;
}

void save_data(unsigned long  storage_id,void* data,int len)
{
    int zone_id  = storage_id >>(BITS_PER_LONG-BITS_PER_BYTE);
    storage_zone *zone = get_zone(zone_id);
    if(zone)
    {
        switch(zone->zone_type)
        {
            case zone_block_type:
            {
                int file_index = ((storage_id << BITS_PER_BYTE) >>(BITS_PER_BYTE))/zone->block_count;
                FILE *stream = find_file_stream(zone,file_index);
                int media_len = sizeof(storage_media)+(zone->block_count/sizeof(long) +1)*sizeof(long);
                if(stream)
                {
                    int offset = ((storage_id << BITS_PER_BYTE) >>(BITS_PER_BYTE))%zone->block_count;
                    offset = offset*zone->block_size;
                    if(0 == file_index)
                    {
                        offset = offset + sizeof(storage_zone);
                    }
                    offset = offset + media_len;
                    fseek(stream,offset,SEEK_SET);
                    fwrite(data,len,1,stream);
                } 
                break;   
            }
            case zone_free_type:
            default:
            printf("zone type err\r\n");
            break;
        }
    }
    return ;
}

void *read_data(unsigned long  storage_id)
{
    int zone_id  = storage_id >>(BITS_PER_LONG-BITS_PER_BYTE);
    storage_zone *zone = get_zone(zone_id);
    char *data = NULL;
    if(zone)
    {
        switch(zone->zone_type)
        {
            case zone_block_type:
            {
                int file_index = ((storage_id << BITS_PER_BYTE) >>(BITS_PER_BYTE))/zone->block_count;
                FILE *stream = find_file_stream(zone,file_index);
                int media_len = sizeof(storage_media)+(zone->block_count/sizeof(long) +1)*sizeof(long);
                if(stream)
                {
                    int offset = ((storage_id << BITS_PER_BYTE) >>(BITS_PER_BYTE))%zone->block_count;
                    offset = offset*zone->block_size;
                    if(0 == file_index)
                    {
                        offset = offset + sizeof(storage_zone);
                    }
                    offset = offset + media_len;
                    data = malloc(zone->block_size);
                    if(data) {
                        fseek(stream,offset,SEEK_SET);
                        fread(data,zone->block_size,1,stream);
                        return data;
                    }
                } 
                break;   
            }
            case zone_free_type:
            default:
            printf("zone type err\r\n");
            break;
        }
    }
    return NULL;
}

unsigned long alloc_key_disk(int len)
{
    return alloc_block_from_zone(storage_zone_key_id);
}
unsigned long save_name(char *key)
{
    int len = strlen(key);
    unsigned long  keys_id = alloc_key_disk(len);
    if(keys_id != -1)
    {
        save_data(keys_id,key,len);
        printf("return key_id is 0x%llx\r\n",keys_id);
        return keys_id;
    }
    return -1;
}


void save_bptree_node(node_block *block)
{
    unsigned long node_id = block->node_id;
    save_data(node_id,block,sizeof(node_block));
}
void save_bptree_root_node(node_block *block)
{
    save_bptree_node(block);
    set_bptree_root_id(block->node_id ,block->node_id >>(BITS_PER_LONG-BITS_PER_BYTE));
}
void set_bptree_root_id(unsigned long root_id,unsigned long zone_id)
{
    storage_zone *zone = get_zone(zone_id);
    storage_media *media = NULL;
    FILE *stream = NULL;
    if(zone)
    {
        zone->root_id = root_id;
        media = zone->media_list;
        if(media->file_index != 0)
        {
            printf("error\r\n");
        }
        else
        {
            stream = media->file_handle;
            fseek(stream,0,SEEK_SET);
            fwrite(zone,sizeof(storage_zone),1,stream);        
        }
        
    }
}

unsigned long get_bptree_rootid(unsigned long zone_id)
{
    storage_zone *zone = get_zone(zone_id);
    if(zone)
    {
        return zone->root_id;
    }
    return -1;
}

void save_wireway_block(wireway_block *block)
{
    unsigned long storage_id = block->wireway_id;
    save_data(storage_id,block,sizeof(wireway_block));
}

void free_key_block(unsigned long id)
{
    return ;
}
void free_data_block(unsigned long id)
{
    return;
}


int storage_sys_init()
{
    storage_zone_wireway_bptree_id = reg_storage_zone("save_wireway_bptree_node",zone_block_type,sizeof(node_block)+1,1000);
    storage_zone_key_id    = reg_storage_zone("save_name_cache",zone_block_type,64,1000);
    storage_zone_wireway_id = reg_storage_zone("save_wireway_struct",zone_block_type,sizeof(wireway_block),1000);

    if(storage_zone_wireway_bptree_id == -1 || storage_zone_key_id == -1 || storage_zone_wireway_id == -1)
    {
        return -1;
    }
    if(-1 == 
    return 0;
}
