#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "list.h"
#include "addr_mgr.h"

#define CMD_SIZE  48
#define IP_SIZE   16

typedef struct
{
    struct list_head head;
    char cmd[CMD_SIZE];
    char ip[IP_SIZE];
} SvrAddr;

static LIST_HEAD(g_svrList);

int AddrMgr_Add(const char* cmd, const char* addr)
{
    int ret = 0;
    
    if( cmd && addr )
    {
        char* ip = AddrMgr_Find(cmd);
        
        if( ip )
        {
            ret = !!strcpy(ip, addr);
        }
        else
        {
            SvrAddr* sa = malloc(sizeof(SvrAddr));
            
            if( ret = !!sa )
            {
                strncpy(sa->cmd, cmd, CMD_SIZE);
                strncpy(sa->ip, addr, IP_SIZE);
                
                sa->cmd[CMD_SIZE-1] = 0;
                sa->cmd[IP_SIZE-1] = 0;
                
                list_add((struct list_head*)sa, &g_svrList);
            }
        }
    }
    
    return ret;
}

char* AddrMgr_Find(const char* cmd)
{
    char* ret = NULL;
    
    if( cmd )
    {
        struct list_head* pos = NULL;
        
        list_for_each(pos, &g_svrList)
        {
            SvrAddr* c = (SvrAddr*)pos;
            
            if( strcmp(cmd, c->cmd) == 0 )
            {
                ret = c->ip;
                break;
            }
        }
    }
    
    return ret;
}

void AddrMgr_Remove(const char* cmd)
{
    char* ip = AddrMgr_Find(cmd);
    
    if( ip )
    {
        SvrAddr* sa = container_of(ip, SvrAddr, ip);
        
        list_del((struct list_head*)sa);
        
        free(sa);
    }
}
void AddrMgr_Clear()
{
    while( !list_empty(&g_svrList) )
    {
        struct list_head* sa = g_svrList.next;
        
        list_del(sa);
        
        free(sa);
    }
}


