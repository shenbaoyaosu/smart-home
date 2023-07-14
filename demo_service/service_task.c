#include "service_task.h"
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "tcp_server.h"
#include "local_service.h"
#include "msg_def.h"

typedef struct 
{
    const char* cmd;
    void* data;
    char*(*handler)(void*);
} Handler;

static char* FormatNumber(float n)
{
    char* ret = malloc(16);

    if( ret )
    {
        sprintf(ret, "%.2f", n);
    }

    return ret;
}

static char* Ill_Get_Handler(void* data)
{
    return FormatNumber(Service_GetData().illumination);
}

static char* Tem_Get_Handler(void* data)
{
    return FormatNumber(Service_GetData().temperature);
}

static char* Hum_Get_Handler(void* data)
{
    return FormatNumber(Service_GetData().humidity);
}

static char* Lig_Get_Handler(void* data)
{
    return FormatNumber(Service_GetData().light);
}

static char* Lig_Set_Handler(void* data)
{
    char* ret = malloc(4);

    if( ret && Service_SetLight(!!data) )
    {
        strcpy(ret, data ? "on" : "off");
    }

    return ret;
}

static Handler g_handler[] = 
{
    {"Ill_Get",       NULL,      Ill_Get_Handler},
    {"Tem_Get",       NULL,      Tem_Get_Handler},
    {"Hum_Get",       NULL,      Hum_Get_Handler},
    {"Lig_Get",       NULL,      Lig_Get_Handler},
    {"Lig_Set_On",    (void*)1,  Lig_Set_Handler},
    {"Lig_Set_Off",   (void*)0,  Lig_Set_Handler},
};

static const int g_size = sizeof(g_handler)/sizeof(*g_handler); 

static char* TouchService(const char* cmd)
{
    char* ret = NULL;
    int i = 0;

    for(i=0; i<g_size; i++)
    {
        if( strcmp(cmd, g_handler[i].cmd) == 0 )
        {
            ret = g_handler[i].handler(g_handler[i].data);
            break;
        }
    }

    return ret;
}

static void EventListener(TcpClient* client, int evt)
{
    if( evt == EVT_CONN )
    {
        printf("%s : CONN - %p...\n", __FUNCTION__, client);
    }
    else if( evt == EVT_DATA )
    {
        Message* msg = NULL;

        printf("%s : DATA - %p...\n", __FUNCTION__, client);

        msg = TcpClient_RecvMsg(client);

        if( msg && (msg->type == TYPE_TOUCH) )
        {
            printf("%s : msg = %p\n", __FUNCTION__, msg);
            printf("%s : msg->type = %d\n", __FUNCTION__, msg->type);
            printf("%s : msg->cmd = %d\n", __FUNCTION__, msg->cmd);
            printf("%s : msg->payload = %s\n", __FUNCTION__, msg->payload);

            char* resp = TouchService((char*)msg->payload);

            free(msg);

            if( resp )
            {
                msg = Message_New(TYPE_RESPONSE, 0, 0, 0, resp, strlen(resp)+1);
            }
            else
            {
                const char* m = "No Service";

                msg = Message_New(TYPE_RESPONSE, 0, 0, 0, m, strlen(m)+1);
            }

            free(resp);
        }
        else
        {
            const char* err = "Invalid touch request...";

            msg = Message_New(TYPE_RESPONSE, 0, 0, 0, err, strlen(err)+1);
        }

        if( msg )
        {
            TcpClient_SendMsg(client, msg);
        }
        else
        {
            Message err = {TYPE_ERROR};
            
            TcpClient_SendMsg(client, &err);
        }

        free(msg);
    }
    else if( evt == EVT_CLOSE )
    {
        printf("%s : CLOSE - %p...\n", __FUNCTION__, client);
    }
}

void* Service_Task(const char* arg)
{
    TcpServer* server = TcpServer_New();

    printf("%s : enter service task...\n", __FUNCTION__);

    Service_Init();

    if( server )
    {
        printf("%s : server = %p\n", __FUNCTION__, server);

        TcpServer_SetListener(server, EventListener);
        TcpServer_Start(server, 8888, 10);
        TcpServer_DoWork(server);
    }

    return arg;
}