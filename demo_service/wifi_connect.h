
#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

int Wifi_Init(void);
int Wifi_Connect(const char* id, const char* pwd);
int Wifi_Start(void);
int Wifi_IsOk(void);
void Wifi_Stop(void);
char* Wifi_IpAddr(void);

#endif



