#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wireway.h"
#include "func.h"
#include "bptree.h"
#include "storage_sys.h"
void list_wireway_tree(node *nd);
wireway_fib* Alloc_fib();
wireway *load_wireway_node_no_fib();

node *wirewayTree = NULL;
wireway *wirewayRestoreList = NULL;

void insert_wireway_restore_list(wireway *w)
{
    if(wirewayRestoreList)
    {
        w->next = wirewayRestoreList;
    }
    wirewayRestoreList = w;
}
wireway *find_wireway_restore_list(char *name)
{
    wireway *w = wirewayRestoreList;
    while(w)
    {
        if(0 == strcmp(w->name,name))
        {
            return w;
        }
        w = w->next;
    }

    return NULL;
}

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
int wireway_tree_empty()
{
    if(wirewayTree)
    return 0;

    return 1;

}
wireway *get_wireway_in_mem(char *name)
{
    wireway *w = find_wireway_restore_list(name);
    if(NULL == w)
    {
        if(is_data_in_mem(wirewayTree,name))
        {
            return lookup_wireway(name);
        }
    }
    return w;
}

point *get_peer_by_index(wireway *srcw,int index)
{
    struct list_head *pos;
    int i = 0;
    list_for_each(pos,&srcw->point_list)
    {
        if(i++ == index)
        {
            if(pos->node_type != point_peer && pos->node_type != point_bridge_peer)
            {
                return NULL;
            }   
            else
            {
                point *p = list_entry(pos,point,list);
                return p;
            }
        }
    }

    return NULL;
}
bridge_point *get_bridge_point_by_index(wireway *srcw,int index)
{

    struct list_head *pos;
    int i = 0;
    list_for_each(pos,&srcw->point_list)
    {
        if(i++ == index)
        {
            if(pos->node_type != point_bridge)
            {
                return NULL;
            }   
            else
            {
                bridge_point *b = list_entry(pos,bridge_point,list);
                return b;
            }
        }
    }

    return NULL;
}

bridge_slave *get_bridge_slave_by_index(wireway *srcw,int index)
{

    struct list_head *pos;
    int i = 0;
    list_for_each(pos,&srcw->point_list)
    {
        if(i++ == index)
        {
            if(pos->node_type != point_bridge_slave)
            {
                return NULL;
            }   
            else
            {
                bridge_slave *slave = list_entry(pos,bridge_slave,list);
                return slave;
            }
        }
    }
    return NULL;
}

restore_fib_by_point(point *p,wireway *dstw)
{
    struct list_head *pos;
    int index = p->index;
    int i = 0;
    point *tmp;
    wireway *srcw= p->wire;
    wireway_fib *fib;

    list_for_each(pos,&dstw->point_list)
    {
        if(srcw == dstw)
        {
            if(i <= index)
            {
                i++;
                continue;
            }
        }    
        switch(pos->node_type)
        {
            case point_peer:
            case point_bridge_peer:
            { 
                tmp = list_entry(pos,point,list);
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
            case point_bridge_slave:
            case point_joint:
            break;
            
            default: printf("case default \r\n");

        }
        i++;
    }

}

int restore_wireway_fib(wireway *fromw,wireway *tow)
{
    struct list_head *pos;
    int index = 0;
    point *tmp;
    wireway_fib *fib;
    bridge_point *b;

    list_for_each(pos,&tow->point_list)
    {        
        switch(pos->node_type)
        {
            case point_peer:
            case point_bridge_peer:
            {                
                tmp = (point*)list_entry(pos,point,list);
                restore_fib_by_point(tmp,tow);
                break;
            }
            case point_bridge:
            {
                b = list_entry(pos,bridge_point,list);
                if(b->bridge_slave.wire == fromw)
                {
                    break;
                }
                if(-1 == b->location_peer_index)
                {
                    point *peer_point = get_peer_by_index(b->bridge_slave.wire,b->bridge_slave.point_index);
                    restore_fib_by_point(peer_point,tow);
                }
                restore_wireway_fib(tow,b->bridge_slave.wire);               

                break;
            }
            
            case point_bridge_slave:
            {
                bridge_slave *slave = list_entry(pos,bridge_slave,list);
                b = list_entry(slave,bridge_point,bridge_slave);
                if(b->wire == fromw)
                {
                    break;
                }
                point *p = get_peer_by_index(tow,slave->point_index);
                restore_fib_by_point(p,b->wire);
                restore_wireway_fib(tow,b->wire);
                break;
            }

            case point_joint:
            {
                
                break;
            }


            default: printf("in line %d,case default \r\n",__LINE__);break;

        }
    }

    return 0;
}

wireway *restore_relate_wireway_node(char *name)
{
    node *leaf;
    int i;
    leaf = find_leaf(wirewayTree,name);
    if(leaf == NULL)
    {
        return NULL;
    }
    
    for(i = 0; i <leaf->num_keys && strcmp(leaf->keys[i],name) != 0;i++);
    
    if(i == leaf->num_keys)
    return NULL;

    if(NULL == leaf->pointers[i])
    {
        leaf->pointers[i] = load_wireway_node_no_fib(leaf->block->pointers_id[i]);
    }
    return NULL;
}

/* this function called by bptree find ,or print tree,but this function may loadanother wireway ,the relate wire be loaded by another func,the return wireway be assigned to bptree,but the relate wire must insert to the bptree*/

wireway *load_wireway_node_no_fib(unsigned long wireway_id)
{
    int i ,j;
    int peer_num = 0;
    int need_load_relate_wireway = 0;
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
            w->state = wire_restoring;
            w->point_num = block->point_num;
            insert_wireway_restore_list(w);            
            INIT_LIST_HEAD(&w->point_list);
            list = &w->point_list;            
            for(i = 0; i < block->point_num ; i++)
            {                    
                point_desc_block *pblock = &block->point_block[i];
                switch(pblock->type)    
                {
                    case point_peer:
                    case point_bridge_peer:
                    {
                        p = (point*)malloc(sizeof(point));
                        if(NULL == p) return NULL;
                        INIT_LIST_HEAD(&p->fib); 
                        set_list_type(&p->list,point_peer);
                        
                        p->dest = pblock->node_desc.peer.dest;
                        p->addr = pblock->node_desc.peer.addr;
                        p->state = pblock->node_desc.peer.state;
                        p->index = pblock->index;
                        p->type  = pblock->type;
                        p->wire = w;
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
                    }
                    case point_bridge:
                    {
                        unsigned long dst_wireway_name_id = pblock->node_desc.bmaster.peer_wireway_name_id;
                        char *peer_name = read_data(dst_wireway_name_id);
                        wireway *peer_wire = NULL;
                        bridge_point *b = NULL;
                        int slave_point_index = pblock->node_desc.bmaster.slave_point_index;
                        if(peer_wire = get_wireway_in_mem(peer_name)){
                               bridge_slave *slave = get_bridge_slave_by_index(peer_wire,slave_point_index);
                               b = list_entry(slave,bridge_point,bridge_slave);
                        }else {
                               b = malloc(sizeof(bridge_point));
                               need_load_relate_wireway = 1;
                        }
                        b->type = pblock->type;
                        b->index = pblock->index;
                        b->dest = pblock->node_desc.bmaster.dest;
                        b->location_peer_index = pblock->node_desc.bmaster.location_peer_index;
                        b->slave_point_index = pblock->node_desc.bmaster.slave_point_index;
                        b->wire = w;                            
                        set_list_type(&b->list,point_bridge);                           
                        list_add(&b->list,list);
                        list = &b->list;
                        if(need_load_relate_wireway)
                        restore_relate_wireway_node(peer_name);
                        break;

                    }
                    
                    case point_bridge_slave:
                    {
                        unsigned long dst_wireway_name_id = pblock->node_desc.bslave.master_wireway_name_id;
                        char *master_name = read_data(dst_wireway_name_id);
                        int bridge_index = pblock->node_desc.bslave.bridge_index;
                        wireway *master_wire = NULL;
                        bridge_point *b= NULL;
                        if(master_wire = get_wireway_in_mem(master_name))
                        {
                            b = get_bridge_point_by_index(master_wire,bridge_index);
                        }
                        else
                        {
                            b = malloc(sizeof(bridge_point));
                            need_load_relate_wireway = 1;
                        }
                        if(b)
                        {
                            set_list_type(&b->bridge_slave.list,point_bridge_slave);
                            list_add(&b->bridge_slave.list,list);
                            b->bridge_slave.type = pblock->type;
                            b->bridge_slave.index = pblock->index;
                            b->bridge_slave.point_index = pblock->node_desc.bslave.point_index;
                            b->bridge_slave.wire = w;
                            list = &b->bridge_slave.list;
                            if(need_load_relate_wireway)
                            {
                                restore_relate_wireway_node(master_name);
                            }
                        }


                        break;
                    }
                    
                    default : printf("error in line %d,error type %d\r\n",__LINE__,pblock->type); break;
                }    
            }
            return w;            
        }     

    }
    return NULL;
}

wireway *load_wireway_node(unsigned long wireway_id)
{
    wireway *w = load_wireway_node_no_fib(wireway_id);
    if(w)
    {
        restore_wireway_fib(w,w);
        return w;
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
        memset(block,0, sizeof(wireway_block));
        block->name_id = -1; /*init name id*/
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

int save_wireway(wireway *srcw)
{
    struct list_head *pos;
    int point_count = 0;
    wireway_block *block = srcw->block;
    wireway_block *tmp = block;
    block->state = srcw->state;
    block->point_num = srcw->point_num;
    
    if(block->name_id == -1)
    {
        block->name_id = save_name(srcw->name);
    }
    if(-1 == block->name_id){
        return 1;
    }

    list_for_each(pos,&srcw->point_list)
    {
        
        if(point_count++ > MAX_POINT_NUM_PER_BLOCK)
        {
            block = (wireway_block*) malloc(sizeof(wireway_block));
            memset(block,0,sizeof(wireway_block));
            block->wireway_id = -1;
            block->wireway_id = alloc_wireway_block();
            if(block->wireway_id == -1)
            {
                return 1;
            }
            tmp->next_block.next_block_ptr = block;
            tmp = block;
            point_count = point_count - MAX_POINT_NUM_PER_BLOCK;
        }
        
        point_desc_block *desc = &tmp->point_block[point_count -1];
        desc->index = point_count - 1;
        switch(pos->node_type)
        {
            case point_peer:
            case point_bridge_peer:
            {
                point *p = list_entry(pos,point,list);
                desc->type  = p->type;
                desc->node_desc.peer.state = p->state;
                desc->node_desc.peer.dest = p->dest;
                desc->node_desc.peer.addr = p->addr;
                break;
            }
            case point_bridge:
            {
                bridge_point *b = list_entry(pos,bridge_point,list);
                desc->type  =  b->type;
                desc->node_desc.bmaster.location_peer_index = b->location_peer_index;
                desc->node_desc.bmaster.peer_wireway_name_id = b->bridge_slave.wire->block->name_id;
                desc->node_desc.bmaster.slave_point_index = b->slave_point_index;
                desc->node_desc.bmaster.dest = b->dest;
                break;
            }
            case point_bridge_slave:
            { 
                bridge_slave *slave = list_entry(pos,bridge_slave,list);
                bridge_point *b = list_entry(slave,bridge_point,bridge_slave);
                desc->type =  slave->type;
                desc->node_desc.bslave.master_wireway_name_id = b->wire->block->name_id;
                desc->node_desc.bslave.point_index = slave->point_index;
                desc->node_desc.bslave.bridge_index = b->index;
                break;
            }
            case point_joint:
                printf("case point_joint \r\n");
                break;
                
            default:
                printf("default %d \r\n",pos->node_type);
                break;

        }

    }
    save_wireway_block(srcw->block);
    return 0;
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
        p->dest = point_in | point_out;
        p->addr  =0xEFEFEFEF;
        p->state = point_idle ;
        w->peer[i] = p;
        p->type = point_peer;
        p->index = i;

    }
    w->point_num = 2;    
    save_wireway(w);

    return 0;
}

point* get_unused_peer(wireway *w)
{
    
    int i;
    for(i = 0 ;i < 2; i++)
    {
        if(point_idle == w->peer[i]->state)
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
    bridge_slave *slave = NULL;

    if(p->type != point_bridge_peer)
    {
        printf("error point type \r\n");
        return NULL;
    }
    
    if(p == p->wire->peer[0])
    {
        slave = list_entry(p->list.prev,bridge_slave,list);
    }
    else if(p == p->wire->peer[1])
    {
        slave = list_entry(p->list.next,bridge_slave,list);
    }
    else
    {
        return NULL;
    }
     
    b = list_entry(slave,bridge_point,bridge_slave);
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
                if(-1 == b->location_peer_index)
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
                    if(b->bridge_slave.wire != p->wire)
                    {
                        tmp = get_peer_by_index(b->bridge_slave.wire,b->bridge_slave.point_index);     
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

#if 0
void insert_restore_wireway_tree(wireway *w)
                               need_load_relate_wireway = 1;
{
    if(NULL == wirewayRestoreTree)
    {
        wirewayRestoreTree = make_new_tree(w->name,w);
    }
    else
    {
        wirewayRestoreTree = insert(wirewayRestoreTree,w->name,w);
    }
}
#endif

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
        point * p = get_peer_by_index(b->bridge_slave.wire,b->bridge_slave.point_index);  
        p->addr = peer1->addr;
        b->location_peer_index = peer1->index;
    }
    else
    {
        point * p = get_peer_by_index(b->bridge_slave.wire,b->bridge_slave.point_index);
        p->addr = peer0->addr;
        b->location_peer_index = peer0->index;
    }    
    return;
}

void assign_point_index(wireway *w)
{
    struct list_head *pos;
    int index = 0;
     
    list_for_each(pos,&w->point_list)
    {
        switch(pos->node_type)
        {
            case point_peer:
            case point_bridge_peer:
            {
                point *p = list_entry(pos,point,list);
                p->index = index;
                break;
            } 
            
            case point_bridge:
            {
                bridge_point *b = list_entry(pos,bridge_point,list);
                b->index = index;
                break;
            }
            case point_bridge_slave:
            {
                bridge_slave *slave = list_entry(pos,bridge_slave,list);
                slave->index = index;
                break;
            }
            case point_joint:
            {
                joint_point *j =  list_entry(pos,joint_point,list);
                j->index = index;
                break;
            }

            default: printf("point type error \r\n"); break;

        }
        index++;
    }
    w->point_num = index;
}

void print_point_type(int type)
{
    switch(type)
    {
        case point_peer:
            printf("point type :point_peer\r\n");
            break;
        case point_bridge_peer:
            printf("point type :point_bridge_peer\r\n");
            break;
        case point_bridge:
            printf("point type :point_bridge\r\n");
            break;
        case point_joint:
            printf("point type :point_joint\r\n");
            break;
        case point_bridge_slave:
            printf("point type :point_bridge_slave\r\n");
            break;
        default:
            printf("point type :error type\r\n");
            break;
    }
}

void print_bridge_point(bridge_point *b)
{
    wireway *srcw = b->wire;
    wireway *dstw = b->bridge_slave.wire;
    printf("point index :%d\r\n",b->index);
    print_point_type(b->type);
    printf("bridge peer wireway name is %s \r\n",dstw->name);
    printf("bridge slave point index is %d\r\n",b->slave_point_index);
    
    if(b->location_peer_index != -1)
    {
        printf("bridge point addr is assigned by src wireway \r\n");
        printf("bridge point assign peer index %d\r\n",b->location_peer_index);
    }
    else
    {
        printf("bridge point addr is assiged default \r\n");
    }

    printf("\r\n");
}
void print_bridge_slave(bridge_slave *slave)
{
    bridge_point *b = list_entry(slave,bridge_point,bridge_slave);
    printf("point index :%d\r\n",slave->index);
    print_point_type(slave->type); 
    printf("point bridge to wireway %s\r\n",b->wire->name);
    printf("\r\n");
}

void print_peer_point(point *p)
{   
    struct list_head *fib;  
    wireway_fib *f;

    printf("point index :%d\r\n",p->index);
    print_point_type(p->type);
    printf("point state: %d\r\n",p->state);
    printf("point addr:0x%x\r\n",p->addr);

    list_for_each(fib,&p->fib)
    {
        f = list_entry(fib,wireway_fib,list);
        printf("fib entry: 0x%x to 0x%x\r\n",f->src,f->dst);
    }

    printf("\r\n");
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
    printf("-----------------------------------------\r\n");
    printf("wireway name %s\r\n",w->name);
    printf("wireway state is %d\r\n",w->state);
    printf("wireway point num is %d\r\n\r\n",w->point_num);

    list_for_each(pos,&w->point_list)
    {          
        switch(pos->node_type)
        {
            case point_peer:
            case point_bridge_peer:
                p = list_entry(pos,point,list);
                print_peer_point(p);
                break;
            case point_bridge:
            {     
                bridge_point *b = list_entry(pos,bridge_point,list); 
                print_bridge_point(b);
               // printf("bridge point !!\r\n");
                break;
            }
            case point_bridge_slave:
            {    bridge_slave *slave = list_entry(pos,bridge_slave,list);
                print_bridge_slave(slave);
               // printf("bridge point slave !!\r\n");
                break;
            }
            case point_joint:
                printf("point joint \r\n");
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
