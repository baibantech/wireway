#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "bptree.h"
#include "bitmap.h"
#include "wireway.h"

#define block_size sizeof(node)
#define node_num 1000
#define index_file_size  node_num*block_size
#define zone_index_mask (0xFFULL << (BITS_PER_LONG - BITS_PER_BYTE))

enum zone_type
{
    bptree_type,
    key_type,
    wireway_type, 
};



typedef struct file_header_tag
{
    int node_count;
    int root_id;
    long bitmap[(node_num/sizeof(long))+1];       
}file_head;

typedef struct storage_zone_tag
{
    char zone_name[32];
    int  zone_id;
    int  zone_type;
    int  size;
    int  total_count;
    int  used_count;
    file_head *head;
    FILE *file;
    struct storage_zone_tag *next;
}storage_zone;

storage_zone zone_list[] =
{
    {.zone_name = "storage_list.mdf",.zone_id = 0,.zone_type = bptree_type,.size = sizeof(node_block),.total_count = 1000 ,.used_count = 0,.file = NULL,.next = NULL},
    {.zone_name = "storage_key.mdf",.zone_id = 0,.zone_type = key_type,.size = 32,.total_count = 1000 ,.used_count = 0,.file = NULL,.next = NULL},
    
    {.zone_name = "storage_wireway.mdf",.zone_id = 0,.zone_type = wireway_type,.size = sizeof(wireway_block),.total_count = 2000 ,.used_count = 0,.file = NULL,.next = NULL}
};

unsigned long alloc_zone_block(storage_zone *zone,unsigned long idx);
int disk_storage_init()
{
    FILE *f = NULL;
    int size = 0;
    int i = 0;
    int fd = 0;
    int res =1;
    file_head *head;
    
    for(i = 0 ; i < sizeof(zone_list)/sizeof(storage_zone) ; i++)
    {

        head = malloc(sizeof(file_head));
        if(!head)
        {
            return 1;
        }

        if(access(zone_list[i].zone_name,0)!= 0)
        {
            f = fopen(zone_list[i].zone_name,"w+");
            if(f)
            {
                fd = fileno(f);
                int res =posix_fallocate(fd,0,zone_list[i].total_count *zone_list[i].size +sizeof(file_head));
                printf("result is %d\r\n",res);
                memset(head,0,sizeof(file_head));
                head->root_id = -1;
                fwrite(head,sizeof(file_head),1,f);
                fclose(f);
            }
            else
            {
                return 1;
            }
        }
      
        f = NULL;
        f = fopen(zone_list[i].zone_name,"rb+");
        if(f)
        {
            fd = fileno(f);
            zone_list[i].zone_id = fd;
            fread(head,sizeof(file_head),1,f);
            zone_list[i].head = head;
            zone_list[i].file = f;            
        }
        else
        {
                return 1;
        }
               
    }        
    return 0;

}
void disk_storage_exit()
{
   // if(index_file_stream)
   // fclose(index_file_stream);
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
