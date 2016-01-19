#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wireway.h"
#include "func.h"
#include "bptree.h"
#include "save_func.h"
void list_wireway_tree(node *nd);

node *wirewayTree = NULL;

int GetStringHash(char *name)
{
    int hashval = 0;
    while(*name != '\0')
    {
        hashval = *name + hashval*31;
        name++;
    }
    return hashval;
}

void wireway_tree_restore()
{
    wirewayTree = restore_bptree_root();
    print_tree(wirewayTree);
    return;
}


wireway *restore_wireway_inst(wireway_block *block)
{
    






}

wireway *load_wireway_node(unsigned long wireway_id)
{
    int i ,j;
    int peer_num = 0;
    point *p = NULL;
    struct list_head *list = NULL; 
    wireway_block *block = (wireway_block*)read_data(wireway_id);
    if(block)
    {
        wireway *w = malloc(sizeof(wireway));
        if(w)
        {
            w->name = read_data(block->name_id);
            w->block = block;
            w->state = block->state;
            INIT_LIST_HEAD(&w->point_list);
            list = &w->point_list;            
            for(i = 0; i < block->point_num ; i++)
            {                    
                point_desc_block *pblock = &block->point_block[i];
                switch(pblock->type)    
                {
                    case point_peer:

                    p = (point*)malloc(sizeof(point));
                    if(NULL == p) return 1;
                    INIT_LIST_HEAD(&p->fib); 
                    set_list_type(&p->list,point_peer);
                    
                    p->dest = block->point_block[i].dest;
                    p->addr = block->point_block[i].addr;
                    p->state = block->point_block[i].state;
                    if(0 == peer_num)
                    {   
                        w->peer[0] = p;
                    }
                    else
                    {
                        w->peer[1] = p;
                    }
                     
                    peer_num++;
                    list_add(&p->list,list);
                    list = &p->list;
                    break;
                    
                    default : printf("error in line %d\r\n",__LINE__); break;
                }    

            }
            return w;            
        }     

    }
    return NULL;
}


wireway *alloc_wireway_inst()
{
    wireway_block *block;
    wireway *w = malloc(sizeof(wireway));
    if(w)
    {
        memset(w,0,sizeof(wireway));
        block = malloc(sizeof(wireway_block));
        if(block)
        {
            block->wireway_id = alloc_wireway_block();
            if(block->wireway_id != -1)
            {
                 w->block = block;
                 return w;
            }
            else
            {
                free(w);
                free(block);
            }
        }
        else
        {
            free(w);
        }
    } 
        
    return NULL;
}

int init_wireway(wireway *w,char *name)
{
    point *p;
    int  i = 0;
    wireway_block *block = w->block;
    
    w->name = malloc(strlen(name)+1);
    if(NULL == w->name)
    {
        return 1;
    }
    strcpy(w->name,name);
    w->next = NULL;
    w->state = wire_init;
    INIT_LIST_HEAD(&w->point_list);
    
    block->type = 0;
    block->point_num = 2;
     

    for(i =0; i < 2 ; i++)
    { 
        p = (point*)malloc(sizeof(point));
        if(NULL == p) return 1;
        INIT_LIST_HEAD(&p->fib); 
        set_list_type(&p->list,point_peer);
        if(0 == i)
        {   
            list_add(&p->list,&w->point_list);
        }else {

            list_add(&p->list,&w->peer[i-1]->list);
        }
        
        p->wire = w;
        p->dest = 3;
        p->addr  =0xEFEFEFEF;
        p->state =0;
        w->peer[i] = p;
        p->type = point_peer;

        block->point_block[i].type = point_peer;
        block->point_block[i].dest = 3;
        block->point_block[i].addr = 0xEFEFEFEF;
        block->point_block[i].state = 0;    
    }
        
    block->name_id = save_name(name);
    if(block->name_id != -1){
        save_wireway_block(block);
    }
    else {
        return 1;
    }
    return 0;
}

point* get_unused_peer(wireway *w)
{
    
    int i;
    for(i = 0 ;i<2; i++)
    {
        if(0 == w->peer[i]->state)
        {
            return w->peer[i];   
        }
    }
    return NULL;
}


wireway_fib* Alloc_fib()
{
    wireway_fib *p = (wireway_fib*) malloc(sizeof(wireway_fib));
    memset(p,0,sizeof(wireway_fib));
    return p;
}

bridge_point *get_relate_bridge(point *p)
{
    bridge_point *b = NULL;

    if(p->type != point_bridge_peer)
    {
        printf("error point type \r\n");
        return NULL;
    }
    
    if(p == p->wire->peer[0])
    {
        b = list_entry(p->list.prev,bridge_point,bridge);

    }
    else if(p == p->wire->peer[1])
    {
        b = list_entry(p->list.next,bridge_point,bridge);
    }
    else
    {
        return NULL;
    }
    
    return b;

}

void update_wireway_fib(point *p,wireway *srcw)
{
    struct list_head *pos;
    point *tmp;
    wireway_fib *fib;
    bridge_point *b;

    list_for_each(pos,&srcw->point_list)
    {
        switch(pos->node_type)
        {
            case point_bridge_peer:
            {                
                tmp = list_entry(pos,point,list);
                                
                if(!tmp->state)
                {
                    printf("error point state \r\n");
                    break;
                }
 
                b =  get_relate_bridge(tmp);
                if(NULL == b->location_peer)
                {
                    update_wireway_fib(tmp,b->wire);
                }
                
                if(tmp != p)
                {
                    if((p->dest & point_in)&&(tmp->dest & point_out)) 
                    {
                        fib = Alloc_fib();
                        fib->src = tmp->addr;
                        fib->dst = p->addr;
                        list_add(&fib->list,&tmp->fib);
                    }
                    if((p->dest &point_out)&&(tmp->dest & point_in)) {
                        fib = Alloc_fib();
                        fib->dst = tmp->addr;
                        fib->src = p->addr;
                        list_add(&fib->list,&p->fib);
                    }
                }

                break;
            }           

            case point_peer: 
            {
                tmp = list_entry(pos,point,list);
                if(!tmp->state || p == tmp)
                {
                    break;
                }
                  
                if((p->dest & point_in)&&(tmp->dest & point_out)) 
                {
                    fib = Alloc_fib();
                    fib->src = tmp->addr;
                    fib->dst = p->addr;
                    list_add(&fib->list,&tmp->fib);
                }
                if((p->dest &point_out)&&(tmp->dest & point_in)) {
                    fib = Alloc_fib();
                    fib->dst = tmp->addr;
                    fib->src = p->addr;
                    list_add(&fib->list,&p->fib);
                }
                break;

            }
            
            case point_bridge:
            {    
                b = list_entry(pos,bridge_point,list);
                if(b->wire == p->wire)
                {
                    printf("error bridge point\r\n");
                }
                else 
                {
                    if(b->peer->wire != p->wire)
                    {
                        tmp = b->peer;     
                        if((p->dest & point_in)&&(tmp->dest & point_out)) 
                        {
                            fib = Alloc_fib();
                            fib->src = tmp->addr;
                            fib->dst = p->addr;
                            list_add(&fib->list,&tmp->fib);
                        }
                        if((p->dest &point_out)&&(tmp->dest & point_in)) {
                            fib = Alloc_fib();
                            fib->dst = tmp->addr;
                            fib->src = p->addr;
                            list_add(&fib->list,&p->fib);
                        }

                    }
                    
                }
                break;
            }
            case point_joint:
            {
                p = list_entry(pos->next,point,list);
                break;
            }
            default:
                break;
        }
    }
}
void insert_wireway_tree(wireway *w)
{
    if(NULL == wirewayTree)
    {
        wirewayTree = make_new_tree(w->name,w);
    } 
    else
    {
        wirewayTree = insert(wirewayTree,w->name,w);
    }
}

int make_insert_value(struct list_head *list,wireway *w)
{
    return 1;
}


void insert_bridge_point(bridge_point *b,wireway *src)
{  
    struct list_head *pos;
    point *insert = NULL;
    point *tmp = NULL;
    int insert_value = 0;
    int tmp_value = 0;

    list_for_each(pos,&src->point_list)
    {
        if(point_peer == pos->node_type)
        {
            if(NULL == tmp)
            {
                tmp = list_entry(pos,point,list);
                if(tmp_value = make_insert_value(&b->list,tmp->wire) > insert_value)
                {
                    insert_value = tmp_value;
                    insert =  tmp;
                }   
            }                   
        }
        
        if(point_joint ==  pos->node_type)
        {
            tmp = NULL;
        }
    }
    
    list_add(&b->list,&insert->list);
}


/*bridge point alloc location*/
void assgin_point_location(bridge_point  *b)
{
    wireway *srcw = b->wire;
    point *p = b->peer;



    /*direction*/


    /*load*/


}

int get_node_load(point *p)
{
    return 0;
}


void assgin_bridge_location(bridge_point *b)
{
    struct list_head *pos;
    point *peer0 = NULL;
    point *peer1 = NULL;
    int peer0_prio = 0;
    int peer1_prio = 0;
    

    pos = get_pre_node(&b->list,point_peer);
    if(NULL != pos)
    {
        peer0 = list_entry(pos,point,list);
    }
    
    pos = get_next_node(&b->list,point_peer);
    
    if(NULL != pos)
    {
        peer1 = list_entry(pos,point,list);
    }

/* direction*/
    if(b->dest &point_in)
    {
        if(peer0->dest & point_in)
        {
            peer0_prio += 50;
        }
        
        if(peer1->dest & point_in)
        {
            peer1_prio += 50;
        }        

    }
        
    if(b->dest & point_out)
    {
        
        if(peer0->dest & point_out)
        {
            peer0_prio += 40;
        }
        
        if(peer1->dest & point_out)
        {
            peer1_prio += 40;
        }        

    }
 
    peer0_prio += get_node_load(peer0);
    peer1_prio += get_node_load(peer1);

    if(peer1_prio >= peer0_prio)
    {
        b->peer->addr = peer1->addr;
        b->location_peer = peer1;
    }
    else
    {
        b->peer->addr = peer0->addr;
        b->location_peer = peer0;
    }    

    return;
}

void print_bridge_point(bridge_point *b)
{
    



}

void print_peer_point(point *p)
{   
    struct list_head *fib;  
    wireway_fib *f;

    printf("point state: %d\r\n",p->state);
    printf("point addr:0x%x\r\n",p->addr);

    list_for_each(fib,&p->fib)
    {
        f = list_entry(fib,wireway_fib,list);
        printf("fib entry: 0x%x to 0x%x\r\n",f->src,f->dst);
    }

}

void print_wireway()
{
    list_wireway_tree(wirewayTree);

}
void print_wireway_detail(wireway *w)
{
    struct list_head  *pos;
    struct list_head *fib;
    point *p;
    int i= 0;

    if(NULL == w)
    return;
    printf("wireway name %s\r\n",w->name);
    printf("wireway state is %d\r\n",w->state);
    list_for_each(pos,&w->point_list)
    {          
        switch(pos->node_type)
        {
            case point_peer:
                p = list_entry(pos,point,list);
                printf("point num %d\r\n",i++);
                print_peer_point(p);
                break;
          case point_bridge:      
                printf("bridge point !!\r\n");
                break;
          case point_bridge_slave:
                printf("bridge point slave !!\r\n");
                break;
          default:
                printf("error\r\n");
                break;
            }
        }
}
void list_wireway_tree(node *nd)
{
    int i ;
    if(NULL == nd)
    {
        return;
    }
    if(!nd->is_leaf)
    {
        for(i = 0; i < nd->num_keys;i++)
        {
            if(NULL == nd->pointers[i])
            {
                nd->pointers[i] = load_bptree_node(nd->block->pointers_id[i]);
            }
            list_wireway_tree(nd->pointers[i]);
        }
    }
    else
    {
        
        for(i = 0; i < nd->num_keys;i++)
        {
            wireway *w =(wireway*) nd->pointers[i];
            if(NULL == w)
            {
                w = load_wireway_node(nd->block->pointers_id[i]);
            }
            print_wireway_detail(w);    
        }

    }
}
wireway* lookup_wireway(char *name)
{
    if(NULL != name)
    {
       return (wireway*)find(wirewayTree,name); 
    }
    return NULL;
}

int is_wireway_exist(char *name)
{
    if(NULL != name)
    {
        return is_key_exist(wirewayTree,name);
    }
}


void destroy_wireway(wireway *w)
{
    struct list_head *pos,*n;
    point *p;
    if(NULL != w)
    { 
        list_for_each_safe(pos,n,&w->point_list)
        {
            if(0 == pos->node_type)
            {
                p = list_entry(pos,struct point_tag,list);
                free(p);
            }

        }
        free(w);
    }
}
