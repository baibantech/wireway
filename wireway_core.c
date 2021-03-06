#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wireway.h"
#include "func.h"

wireway* create_wireway(char *name);
int attach_wireway(char *name , int addr);
int bridge_wireway(char *srcname,point *p);

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
wireway *restore_wireway(char *name)
{





}




int attach_wireway(char *name,int addr)
{
    wireway *w = lookup_wireway(name);
    point *p = get_unused_peer(w);
    if(p != NULL)
    {
        p->state = point_active;
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
        save_wireway(w);
 
        return 0;
    }            
    return 1;
}
/* one wireway peer bridge another wireway*/

int bridge_wireway(char *srcname,point *p)
{
    wireway *dstw = lookup_wireway(srcname);
    bridge_point *b; 
    wireway *srcw = p->wire;
    if(!dstw || ((dstw != NULL) && (dstw->state != wire_active)) ) 
    {
        return 1;
    }
    b = (bridge_point*)malloc(sizeof(bridge_point));
    if(NULL == b)
    {
        return 1;
    }
    b->location_peer_index = -1;
    b->dest = p->dest;
    b->wire = dstw;
    p->type = point_bridge_peer;

    set_list_type(&b->bridge_slave.list,point_bridge_slave);
    b->bridge_slave.type = point_bridge_slave;
    b->bridge_slave.wire = srcw;
    if(srcw->peer[0] == p)
    {
        list_add(&b->bridge_slave.list,&srcw->point_list);
    }else if(srcw->peer[1] == p) {
        list_add(&b->bridge_slave.list,&p->list);
    }else {
        printf("error in bridge\r\n");
        return 1;
    }
   
    assign_point_index(srcw);
    b->bridge_slave.point_index = p->index;

    set_list_type(&b->list,point_bridge);
    b->type = point_bridge;

    {

        insert_bridge_point(b,dstw);
        assign_point_index(dstw);
        b->slave_point_index = b->bridge_slave.index; 
        if(p->state == point_idle)
        {
            assgin_bridge_location(b);
            p->state = point_active;
            
            switch(srcw->state)
            {
                case wire_init: 
                {
                    srcw->state = wire_attaching; 
                    break;
                }
                case wire_attaching: 
                {
                    srcw->state = wire_active;
                    update_wireway_fib(p,srcw);
                    break;
                }
                default:printf("wire state error \r\n");break;

            }
        }
        else
        {
            if(srcw->state == wire_active)
            {
                update_wireway_fib(p,dstw);
            }        
        }
    }
    save_wireway(srcw);
    save_wireway(dstw);

}
/*too single direction wireway construct a double direction wireway*/ 
int make_wireway_peer(char *srcname,char *dstname)
{




}


void test_wireway()
{

    point *p;
    wireway *w;
    create_wireway("/chn/name2");
    w =  create_wireway("/chn/name1");
    attach_wireway("/chn/name1",0x1234);
    //attach_wireway("/chn/name1",0xbcbcbcbc);
    attach_wireway("/chn/name1",0x5678);    


    attach_wireway("/chn/name2",0x9012);
    attach_wireway("/chn/name2",0x3456);
    p = get_peer_by_index(w,1);
    bridge_wireway("/chn/name2",p);
    print_wireway();
}


void test_restore_wireway()
{
    restore_wireway("/chn/name1");
    restore_wireway("/chn/name2");
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
   attach_wireway("chn/name1",0x5678);
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
/*
int main(void)
{
    storage_sys_init();
    wireway_tree_restore();
    if(wireway_tree_empty())
    {
        test_wireway2();
    }
    else
    {
        test_restore_wireway();
    } 
    return 0;
}
*/
