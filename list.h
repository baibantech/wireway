#ifndef __LIST__
#define __LIST__
#include <stdio.h>

#define prefetch(x) __builtin_prefetch(x)
#define LIST_HEAD_INIT(name) { &(name),&(name),0}
#define list_entry(ptr,type,member)\
    container_of(ptr,type,member)

#define offsetof(TYPE,MEMBER) ((size_t)&((TYPE *)0)->MEMBER)

#define container_of(ptr,type,member) ( {\
    const typeof( ((type*)0)->member ) *__mptr=(ptr);\
    (type*)( (char*)__mptr - offsetof(type,member) );})

#define list_for_each(pos, head) \
         for (pos = (head)->next; prefetch(pos->next), pos != (head); \
                 pos = pos->next)

#define list_for_each_safe(pos, n, head)\
     for (pos = (head)->next, n = pos->next; pos != (head);\
            pos = n, n = pos->next)

struct list_head
{
    struct list_head *next,*prev;
    char node_type;
};
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void __list_add( struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}
static inline struct list_head* get_next_node(struct list_head *entry,int type)
{
    struct list_head *pos;
    for(pos = entry->next ; pos != entry; pos = pos->next)
    {
        if(pos->node_type == type)
        {
            return pos;
        } 
    }
    return NULL;
}

static inline struct list_head* get_pre_node(struct list_head *entry,int type)
{
    struct list_head *pos = NULL;
    for(pos = entry->prev ; pos != entry ;pos = pos->prev)
    {
        if(pos->node_type == type)
        {
            return pos;
        }

    }
    return NULL;
}
static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}
static inline void set_list_type(struct list_head *head,char type)
{
    head->node_type = type;
}

#endif
