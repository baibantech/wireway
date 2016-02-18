#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bptree.h"
#include "bitmap.h"
#include "wireway.h"
#include "storage_sys.h"


storage_zone_head *zone_head = NULL;
storage_buddy_head *buddy_head = NULL;
int global_zone_id = 0;

unsigned long alloc_zone_block(storage_zone *zone,unsigned long idx);

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
        zone_tmp->used_count = 0;
        if(zone_type == zone_block_type)
        {
            len = sizeof(storage_media)+(zone->block_count/sizeof(long) +1)*sizeof(long);
            media = malloc(len);
            if(media)
            {
                memset(media,0,len);
                media->file_index = 0;
                media->file.block_file.block_free_count = per_media_count;
                media->file.root_id = -1;
                f = fopen(zone_name,"w+");
                if(f)
                {
                    int fd = fileno(f);
                    posix_fallocate(fd,0,len);
                    fwrite(zone,sizeof(storage_zone),1,f);
                    fseek(f,sizeof(storage_zone),SEEK_SET);
                    fwrite(media,len,1,f);
                    if(opened_media_file_count < MAX_OPEN_MEDIA_FILE)
                    {
                        media->file.block_file.file_handle = f;
                        open_media_file_count++;
                    }
                    else
                    {
                        media->file.block_file.file_handle = NULL;
                        fclose(f);
                    }
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
        zone_tmp->zone_id = alloc_zone_id();
        return zone_tmp;
    }
    return NULL;
}


int reg_storage_zone(char *zone_name,int zone_type,int block_size,int per_media_count)
{
    storage_zone_head *zone_head ;
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

    if(NULL == zone_head)
    {
        zone_head = malloc(sizeof(storage_zone_head));
        if(NULL == zone_head) return -1;
        memset(zone_head,0,sizeof(storage_zone_head);
    }
    zone_pre = zone_tmp = zone_head->zone_list;
    while(zone_tmp)
    {
        if(0 = strcmp(zone_tmp->zone_name,zone_name))
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
        zone_head->zone_list = zone_tmp;
    }
    else
    {
        zone_pre->next = zone_tmp;
    }
    zone_head->zone_num++;
    return zone_tmp->zone_id;

free_zone_head:
    free(zone_head);
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
    if(NULL == zone_head)
    {
        return NULL;
    }
    zone = zone_head->zone_list;
    while(zone)
    {
        if(zone->zone_id == zone_id)
        {
            return zone;
        }
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
                    return (block_index +media->file_index *zone->block_count)|(zone->zone_id << (BITS_PER_LONG- BITS_PER_BYTE));            
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

void save_data(unsigned long  id,void* data,int len)
{
    unsigned long  index = (id & zone_index_mask)>>(BITS_PER_LONG-BITS_PER_BYTE);    
    file_head *head = zone_list[index].head;
    unsigned long offset = zone_list[index].size*((id<<BITS_PER_BYTE)>>(BITS_PER_BYTE))+sizeof(file_head);
    FILE *file_stream = zone_list[index].file;
    fseek(file_stream,offset,SEEK_SET);
    fwrite(data,len,1,file_stream);
    return ;   
}
unsigned long alloc_key_disk(int len)
{
    unsigned long  i = 0;
   // unsigned long  index = 0;
    
    for(i = 0;i < sizeof(zone_list)/sizeof(storage_zone);i++)
    {
        if((zone_list[i].zone_type == key_type) && (zone_list[i].size > len))
        {
            return alloc_zone_block(&zone_list[i],i);
        }
    }

    return -1;
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
void save_data(unsigned long  id,void* data,int len)
{
    unsigned long  index = (id & zone_index_mask)>>(BITS_PER_LONG-BITS_PER_BYTE);    
    file_head *head = zone_list[index].head;
    unsigned long offset = zone_list[index].size*((id<<BITS_PER_BYTE)>>(BITS_PER_BYTE))+sizeof(file_head);
    FILE *file_stream = zone_list[index].file;
    fseek(file_stream,offset,SEEK_SET);
    fwrite(data,len,1,file_stream);
    printf("0x%llx\r\n",id);
    printf("0x%llx\r\n",zone_index_mask);
    printf("0x%llx\r\n",(BITS_PER_LONG-BITS_PER_BYTE));
    printf("0x%llx\r\n",(id & zone_index_mask)>>(BITS_PER_LONG-BITS_PER_BYTE));
   return ;   
}

void *read_data(unsigned long id)
{
    unsigned long  index = (id & zone_index_mask)>>(BITS_PER_LONG-BITS_PER_BYTE);    
    file_head *head = zone_list[index].head;
    unsigned long offset = zone_list[index].size*((id<<BITS_PER_BYTE)>>(BITS_PER_BYTE))+sizeof(file_head);
    FILE *file_stream = zone_list[index].file;
   
    char *data = malloc(zone_list[index].size);

    if(data) {
        fseek(file_stream,offset,SEEK_SET);
        fread(data,zone_list[index].size,1,file_stream);
    }
   return data;   
}


unsigned long alloc_zone_block(storage_zone *zone,unsigned long idx)
{
   file_head *head = zone->head;
   unsigned long index = find_next_zero_bit(head->bitmap,node_num,0);
   FILE *file_stream = zone->file; 
   set_bit(index,head->bitmap);

   fseek(file_stream,0,SEEK_SET);
   fwrite(head,sizeof(file_head),1,file_stream);
   printf("block index is %d\r\n",index);
   return index |(idx <<(BITS_PER_LONG -BITS_PER_BYTE));
   
}
   
unsigned long  alloc_bptree_block()
{
    return alloc_zone_block(&zone_list[bptree_type],0);
}

unsigned long alloc_wireway_block()
{

    return alloc_zone_block(&zone_list[wireway_type],wireway_type);
}

void save_bptree_node(node_block *block,int is_root)
{
    unsigned long node_id = block->node_id; 
    file_head *head = zone_list[bptree_type].head;
    unsigned long offset = zone_list[bptree_type].size*node_id+sizeof(file_head);
    FILE *file_stream = zone_list[bptree_type].file;
    fseek(file_stream,offset,SEEK_SET);
    fwrite(block,sizeof(node_block),1,file_stream);
    if(is_root)
    {
       head->root_id = node_id;
       fseek(file_stream,0,SEEK_SET);
       fwrite(head,sizeof(file_head),1,file_stream);
    }
    return ;
}

unsigned long get_bptree_rootid()
{
    file_head *head = zone_list[bptree_type].head;
    return head->root_id;
}
unsigned long get_bptree_dataid(void *data)
{
    wireway *w = (wireway*)data;
    return w->block->wireway_id;
}
unsigned long get_bptree_keyid(void *data)
{
    wireway *w = (wireway*)data;
    return w->block->name_id;
}
void save_wireway_block(wireway_block *block)
{
    unsigned long id = block->wireway_id;
    
    unsigned long  index = (id & zone_index_mask)>>(BITS_PER_LONG-BITS_PER_BYTE);    
    file_head *head = zone_list[index].head;
    unsigned long offset = zone_list[index].size*((id<<BITS_PER_BYTE)>>(BITS_PER_BYTE))+sizeof(file_head);
    FILE *file_stream = zone_list[index].file;
    fseek(file_stream,offset,SEEK_SET);
    fwrite(block,sizeof(wireway_block),1,file_stream);

}

void free_key_block(unsigned long id)
{
    return ;
}
void free_data_block(unsigned long id)
{
    return;
}
