#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "message.h"
#include "tcp_client.h"
#include "udp_point.h"
#include "addr_mgr.h"
#include "utility.h"
#include "msg_def.h"

#define BUF_SIZE 64
#define DIM(a) sizeof(a)/sizeof(*a)

typedef struct 
{
    const char* cmd;
    void(*handler)(const char*);
} Handler;

static void ParseCommand(const char* s)
{
    const char* desc = s;
    const char* ip = s + 64;
    const char* usage = s + 80;
    int i = 0;
    int j = 0;
    int count = 0;
    char** parts = NULL;
    
    printf("%s:\n", desc);
    
    count = CharCount(usage, '\n');
    parts = Malloc2d(char, count, BUF_SIZE);
    count = DivideByChar(usage, '\n', parts, count, BUF_SIZE);
    
    for(i=0; i<count; i++)
    {
        printf("....%s\n", parts[i]);
        
        int c = CharCount(parts[i], ' ') + 1;
        char** cmd = Malloc2d(char, c, BUF_SIZE);
        
        c = DivideByChar(parts[i], ' ', cmd, c, BUF_SIZE);
        
        for(j=1; j<c; j++)
        {
            AddrMgr_Add(cmd[j], ip);
        }
        
        Free2d(cmd);
    }
    
    Free2d(parts);
}

static void Query_Handler(const char* arg)
{
    UdpPoint* udp = UdpPoint_New(8888);
    
    if( udp )
    {
        Message msg = {TYPE_QUERY, 0, 0, 0, 0};
        char remote[16] = {0};
        int port = 0;
        int brd = 1;
        int i = 0;
        
        UdpPoint_SetOpt(udp, SOL_SOCKET, SO_BROADCAST, &brd, sizeof(brd));
        UdpPoint_SendMsg(udp, &msg, "255.255.255.255", 9999);
        
        while( i < 3 )
        {
            if( UdpPoint_Available(udp) != -1 )
            {   
                Message* resp = UdpPoint_RecvMsg(udp, remote, &port);
                
                // process response
                ParseCommand(resp->payload);
                
                free(resp);
                
                i = 0;
            }
            else
            {
                sleep(1);
                i++;
            }
        }
    }
    
    UdpPoint_Del(udp);
}

static void Touch_Handler(const char* arg)
{
    TcpClient* client = TcpClient_New();
    
    if( client && *arg )
    {
        char* ip = AddrMgr_Find(arg);
        
        if( ip )
        {
            Message* msg = Message_New(TYPE_TOUCH, 0, 0, 0, arg, strlen(arg)+1);
            
            if( msg && TcpClient_Connect(client, ip, 8888) )
            {
                if( TcpClient_SendMsg(client, msg) )
                {
                    free(msg);
                    
                    msg = TcpClient_RecvMsg(client);
                    
                    if( msg && (msg->type == TYPE_RESPONSE) )
                    {
                        printf("Response: %s\n", msg->payload);
                    }
                }
                else
                {
                    AddrMgr_Remove(arg);
                }
                
                TcpClient_Close(client);
            }
            
            free(msg);
        }
        else
        {
            printf("Can NOT find service...\n");
        }
    }
    
    TcpClient_Del(client);
}

static Handler g_handler[] = 
{
    {"query", Query_Handler},
    {"touch", Touch_Handler},
};

int main()
{
    char line[BUF_SIZE] = {0};
    char** arg = Malloc2d(char, 2, BUF_SIZE);
    
    printf("Client Demo @ D.T.Software\n");
    
    while( arg )
    {
        printf("Input >>> ");
        
        fgets(line, sizeof(line)-1, stdin);
        
        line[strlen(line)-1] = 0;
        
        *arg[0] = 0;
        *arg[1] = 0;

        if( *line )
        {
            int i = 0;         
            int r = DivideByChar(line, ' ', arg, 2, BUF_SIZE);
            
            for(i=0; (i<DIM(g_handler)) && (r>0); i++)
            {
                if( strcmp(g_handler[i].cmd, arg[0]) == 0 )
                {
                    g_handler[i].handler(arg[1]);
                    break;
                }
            }
        }
    }
    
    Free2d(arg);
    
    return 0;
}

