#include <stdio.h>
#include <pthread.h>
int  get_entity_thread_num()
{
    return 1;
}
int get_wireway_thread_num()
{
    return 1;
}

void entity_thread_main()
{




}
void wireway_thread_main()
{



}
void entity_thread_init()
{
    pthread_t entity_tid;
    int i, ret = 0;
    int num = get_entity_thread_num();
    for(i = 0 ; i < num ; i++)
    {
        ret = pthread_create(&entity_tid,NULL,entity_thread_main,NULL);
        if(ret)
        {
            break;
        }
        
    }
    
}

void wireway_thread_init()
{
    pthread_t wireway_tid;
    int i,ret = 0;
    int num = get_wireway_thread_num();
    for(i = 0; i < num ; i++)
    {
        ret = pthread_create(&wireway_tid,NULL,wireway_thread_main,NULL);
        if(ret)
        {
            break;
        }
    }

}

