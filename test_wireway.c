#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wireway.h"
#include "func.h"

wireway* create_wireway(char *name);
int attach_wireway(char *name , int addr);
int bridge_wireway(char *srcname,point *p);
void disk_storage_init();

wireway* create_wireway(char *name)
{
    wireway *w ;
    point  *p;

    if(0 == strlen(name))
    {
        return NULL;
    }
    
    if(is_wireway_exist(name))
    {
        return NULL;
    }   

    w = alloc_wireway_inst();
    if(NULL == w)
    {
        return NULL;
    }
    
    if(init_wireway(w,name))
    {
        destroy_wireway(w);       
    }
    
    insert_wireway_tree(w);
    
    return w;
}

int attach_wireway(char *name,int addr)
{
    wireway *w = lookup_wireway(name);
    point *p = get_unused_peer(w);
    if(p != NULL)
    {
        p->state = 1;
        p->addr = addr;
        switch(w->state)
        {
            case wire_init: 
            {
                w->state = wire_attaching; 
                break;
            }
            case wire_attaching: 
            {
                w->state = wire_active;
                update_wireway_fib(p,w);
                break;
            }
            default:printf("wire state error \r\n");break;

        }
        w->block->point_block[p->index].state = p->state;
        w->block->point_block[p->index].addr  = p->addr;
        w->block->state = w->state;        
        save_wireway_block(w->block);
 
        return 0;
    }            
    return 1;
}
/* one wireway peer bridge another wireway*/

int bridge_wireway(char *srcname,point *p)
{
    wireway *srcw = lookup_wireway(srcname);
    bridge_point *b; 
    wireway *w = p->wire;
    if(!srcw || ((srcw != NULL) && (srcw->state != wire_active)) ) 
    {
        return 1;
    }
    b = (bridge_point*)malloc(sizeof(bridge_point));
    if(NULL == b)
    {
        return 1;
    }
    
    b->peer = p;
    b->dest = p->dest;
    b->wire = srcw;
    p->type = point_bridge_peer;

    set_list_type(&b->bridge,point_bridge_slave);
    if(p->list.prev == &p->wire->point_list)     
    {
        list_add(&b->bridge,&p->wire->point_list);
    }else {
        list_add(&b->bridge,&p->list);
    }

    set_list_type(&b->list,point_bridge);
    /*
    if(srcw->state != wire_active)
    {
        list_add(&b->list,&srcw->tmp_list);
        return 0;
    }
    else*/
    {

        insert_bridge_point(b,srcw);
    
        if(p->state == 0)
        {
            assgin_bridge_location(b);
            p->state = 1;
            
            switch(w->state)
            {
                case wire_init: 
                {
                    w->state = wire_attaching; 
                    break;
                }
                case wire_attaching: 
                {
                    w->state = wire_active;
                    update_wireway_fib(p,w);
                    break;
                }
                default:printf("wire state error \r\n");break;

            }
        }
        else
        {
            if(w->state == wire_active)
            {
                update_wireway_fib(p,srcw);
            }        
        }
    }

}
/*too single direction wireway construct a double direction wireway*/ 
int make_wireway_peer(char *srcname,char *dstname)
{




}


void test_wireway()
{

    point *p;
    wireway *w;
    create_wireway("/chn/name1");
    w =  create_wireway("/chn/name2");
    attach_wireway("/chn/name1",0xabababab);
    //attach_wireway("/chn/name1",0xbcbcbcbc);
    attach_wireway("/chn/name1",0xcdcdcdcd);    


    attach_wireway("/chn/name2",0xedededed);
    p = get_unused_peer(w);
    bridge_wireway("/chn/name1",p);
    print_wireway();
}




void test_wireway1()
{
   wireway *w = NULL;
   point *p = NULL;
   create_wireway("chn/name1");

    attach_wireway("chn/name1",0x123);

    attach_wireway("chn/name1",0x456);


    w = create_wireway("chn/name2");

    attach_wireway("chn/name2",0x789);

    attach_wireway("chn/name2",0x910);

    bridge_wireway("chn/name1",w->peer[0]);


    w = create_wireway("chn/name3");
    
    attach_wireway("chn/name3",0x134);

    p = get_unused_peer(w);

    bridge_wireway("chn/name2",p);  
   
    print_wireway();


}

void test_wireway2()
{



   create_wireway("chn/name1");
   attach_wireway("chn/name1",0x1234);
   print_wireway();   
   #if 0
   create_wireway("chn/name2");
   create_wireway("chn/name3");
   create_wireway("chn/name4");
   create_wireway("chn/name5");
   create_wireway("chn/name6");
   create_wireway("chn/name7");
   create_wireway("chn/name8");
   create_wireway("chn/name9");
   create_wireway("chn/name10");
   create_wireway("chn/name11");
   create_wireway("chn/name12");
   create_wireway("chn/name13");
   create_wireway("chn/name14");
   create_wireway("chn/name15");
   create_wireway("chn/name16");
   create_wireway("chn/name17");
   create_wireway("chn/name18");
   create_wireway("chn/name19");
   printf("size of long is %d\r\n",(int)sizeof(long));
   printf("size of ptr is %d\r\n",(int)sizeof(void*));
   #endif
   //disk_storage_exit();
}

int main(void)
{
    disk_storage_init();
    wireway_tree_restore();
    test_wireway2();
    return 0;
}
