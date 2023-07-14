#ifndef LOCAL_SERVICE_H
#define LOCAL_SERVICE_H

typedef struct
{
	float    illumination;
	float    humidity;
	float    temperature;
	int      light;
} SvrData;


void Service_Init(void);
const char* Service_GetDesc(void);
const char* Service_GetUsage(void);
SvrData Service_GetData(void);
int Service_SetLight(int on);

#endif


