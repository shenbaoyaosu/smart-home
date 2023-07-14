
#include <stdlib.h>
#include "response_task.h"
#include "udp_point.h"
#include "msg_def.h"
#include "local_service.h"
#include "wifi_connect.h"

#define DESC_SIZE   64
#define ADDR_SIZE   16
#define USAGE_SIZE  256

static void Query_Handler(UdpPoint* udp, Message* msg, const char* remote, int port)
{
    Message* resp = Message_New(TYPE_RESPONSE, 0, 0, 0, NULL, DESC_SIZE + ADDR_SIZE + USAGE_SIZE);

    if( resp )
    {
        strncpy(resp->payload, Service_GetDesc(), DESC_SIZE);
        strncpy(resp->payload + DESC_SIZE, Wifi_IpAddr(), ADDR_SIZE);
        strncpy(resp->payload + DESC_SIZE + ADDR_SIZE, Service_GetUsage(), USAGE_SIZE);

        UdpPoint_SendMsg(udp, resp, remote, port);
    }

    free(resp);
}

void* Response_Task(const char* arg)
{
    UdpPoint* udp = UdpPoint_New(9999);

    printf("%s : enter response task...\n", __FUNCTION__);

    if( udp )
    {
        char remote[16] = {0};
        int port = 0;

        printf("%s : udp = %p\n", __FUNCTION__, udp);

        while( 1 )
        {
            Message* msg = UdpPoint_RecvMsg(udp, remote, &port);

            if( msg && (msg->type == TYPE_QUERY) )
            {
                printf("%s : msg = %p\n", __FUNCTION__, msg);
                printf("%s : remote = %s\n", __FUNCTION__, remote);
                printf("%s : port = %d\n", __FUNCTION__, port);
                printf("%s : msg->type = %d\n", __FUNCTION__, msg->type);
                printf("%s : msg->cmd = %d\n", __FUNCTION__, msg->cmd);

                Query_Handler(udp, msg, remote, port);
            }
            else
            {
                printf("%s : msg is NULL...\n", __FUNCTION__);
            }

            free(msg);
        }
    }

    return arg;
}