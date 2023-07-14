

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_connect.h"
#include "wifi_device.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/ip4_addr.h"
#include "lwip/api_shell.h"
#include "lwip/dhcp.h"

#define WIFI_TIMEOUT 20
#define WLAN_PORT    "wlan0"

static int g_WaitResult = 0;
static WifiEvent g_WifiEventHandler = {0};
static struct netif* g_LwipNetif = NULL;

static void ClearWaitResult(void)
{
    g_WaitResult = 0;
}

static void SetWaitResult(int result)
{
    g_WaitResult = result;
}

static int GetWaitResult(void)
{
    return g_WaitResult;
}

static void ToWait(unsigned int sec)
{
    while( sec-- && !GetWaitResult() )
    {
        sleep(1);
    }
}

static void OnWifiScanStateChanged(int state, int size)
{
    (void)state;
    (void)size;
}

static void OnWifiConnectionChanged(int state, WifiLinkedInfo *info)
{
    (void)info;

    SetWaitResult(state);
}

static void OnHotspotStaJoin(StationInfo *info)
{
    (void)info;
}

static void OnHotspotStateChanged(int state)
{
    (void)state;
}

static void OnHotspotStaLeave(StationInfo *info)
{
    (void)info;
}

int Wifi_Init(void)
{
    g_WifiEventHandler.OnWifiScanStateChanged = OnWifiScanStateChanged;
    g_WifiEventHandler.OnWifiConnectionChanged = OnWifiConnectionChanged;
    g_WifiEventHandler.OnHotspotStaJoin = OnHotspotStaJoin;
    g_WifiEventHandler.OnHotspotStaLeave = OnHotspotStaLeave;
    g_WifiEventHandler.OnHotspotStateChanged = OnHotspotStateChanged;

    return RegisterWifiEvent(&g_WifiEventHandler);
}

int Wifi_Connect(const char* id, const char* pwd)
{
    int ret = WIFI_SUCCESS;

    if( !Wifi_IsOk() )
    {
        ret = id && pwd ? EnableWifi() : ERROR_WIFI_UNKNOWN;
        // ret = (ret == WIFI_SUCCESS) ? && IsWifiActive() ? WIFI_SUCCESS : ERROR_WIFI_NOT_AVAILABLE;

        if( ret == WIFI_SUCCESS )
        {
            WifiDeviceConfig config = {0};
            int result = 0;

            config.securityType = WIFI_SEC_TYPE_PSK;

            strcpy(config.ssid, id);
            strcpy(config.preSharedKey, pwd);

            ret += AddDeviceConfig(&config, &result);
            ret += ConnectTo(result);

            if( ret == WIFI_SUCCESS )
            {
                ClearWaitResult();

                ToWait(WIFI_TIMEOUT);

                ret = (GetWaitResult() > 0) ? WIFI_SUCCESS : ERROR_WIFI_UNKNOWN;
            }
        }
    }

    return ret;
}

int Wifi_Start(void)
{
    int ret = WIFI_SUCCESS;

    if( !Wifi_IsOk() )
    {
        g_LwipNetif = netifapi_netif_find(WLAN_PORT);

        if( g_LwipNetif )
        {
            int i = WIFI_TIMEOUT;

            if( dhcp_start(g_LwipNetif) == ERR_OK )
            {
                while( ((ret = dhcp_is_bound(g_LwipNetif)) != ERR_OK) && i-- )
                {
                    usleep(200 * 1000);
                }
            }

            if( ret != WIFI_SUCCESS )
            {
                Wifi_Stop();
            }
        }
        else
        {
            ret = ERROR_WIFI_UNKNOWN;
        }
    }

    return ret;
}

int Wifi_IsOk(void)
{
    return !!g_LwipNetif;
}

void Wifi_Stop(void)
{
    dhcp_stop(g_LwipNetif);

    g_LwipNetif = NULL;
}

char* Wifi_IpAddr(void)
{
    char* ret = NULL;
    
    if( Wifi_IsOk() )
    {
        ip4_addr_t addr = {0};
        ip4_addr_t mask = {0};
        ip4_addr_t gw = {0};

        netif_get_addr(g_LwipNetif, &addr, &mask, &gw);

        if( (addr.addr != 0) && (addr.addr != -1) )
        {
            ret = ip4addr_ntoa(&addr);
        }
        else
        {
            Wifi_Stop();
        }
    }

    return ret;
}
