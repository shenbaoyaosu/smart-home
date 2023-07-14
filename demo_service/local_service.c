#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_i2c.h"
#include "hi_io.h"
#include "hi_i2c.h"
#include "local_service.h"

#define SHT30_Addr 0x44
#define BH1750_Addr 0x23

static unsigned int IotGpioSetFunc(unsigned int id, unsigned char val)
{
    return hi_io_set_func(id, val);
}

static unsigned int IotI2cWriteRead(unsigned int id, unsigned short deviceAddr, unsigned char* txBuf, unsigned int txLen, unsigned char* rxBuf, unsigned int rxLen)
{
    hi_i2c_data data = {0};

    data.send_buf = txBuf;
    data.send_len = txLen;
    data.receive_buf = rxBuf;
    data.receive_len = rxLen;

    return hi_i2c_writeread(id, deviceAddr, &data);
}

static void Init_BH1750(void)
{
    uint8_t send_data[1] = {0};

    send_data[0] = 0x01;

    IoTI2cWrite(1, (BH1750_Addr << 1)|0x00, send_data, 1);

    usleep(100000);

    send_data[0] = 0x10;

    IoTI2cWrite(1, (BH1750_Addr << 1)|0x00, send_data, 1);
}

static void Init_SHT30(void)
{
    uint8_t send_data[2] = {0x22, 0x36};

    IoTI2cWrite(1, (SHT30_Addr << 1)|0x00, send_data, 2);
}

void Service_Init(void)
{
    IoTGpioInit(14);
    IotGpioSetFunc(14, HI_IO_FUNC_GPIO_14_GPIO);
    IoTGpioSetDir(14, IOT_GPIO_DIR_OUT);

    IoTGpioInit(0);
    IoTGpioInit(1);

    IotGpioSetFunc(0, HI_IO_FUNC_GPIO_0_I2C1_SDA);
    IotGpioSetFunc(1, HI_IO_FUNC_GPIO_1_I2C1_SCL);

    IoTI2cInit(1, 400000);

    usleep(100000);

    Init_BH1750();

    usleep(100000);

    Init_SHT30();
}

const char* Service_GetDesc(void)
{
    return "Environment Service";
}

const char* Service_GetUsage(void)
{
    return "Illumination: Ill_Get\n"
           "Temperature: Tem_Get\n"
           "Humidity: Hum_Get\n"
           "Light: Lig_Get Lig_Set_On Lig_Set_Off\n";
}

SvrData Service_GetData(void)
{
    SvrData ret = {0};
    uint8_t send_data[2] = {0xE0, 0x00};
    uint8_t recv_data[6] = {0};

    IoTI2cRead(1, (BH1750_Addr << 1)|0x01, recv_data, 2);

    ret.illumination = (float)((recv_data[0] << 8) + recv_data[1]) / 1.2;

    IotI2cWriteRead(1, (SHT30_Addr << 1)|0x00, send_data, 2, recv_data, 6);

    {
        uint16_t u16sT = ((uint16_t)recv_data[0] << 8) | recv_data[1];

        u16sT &= ~0x0003;

        ret.temperature = (175 * (float)u16sT / 65535 - 45);
    }

    {
        uint16_t u16sRH = ((uint16_t)recv_data[3] << 8) | recv_data[4];

        u16sRH &= ~0x0003;

        ret.humidity = (100 * (float)u16sRH / 65535);
    }

    IoTGpioGetOutputVal(14, &ret.light);

    return ret;
}

int Service_SetLight(int on)
{
    return (IoTGpioSetOutputVal(14, on) == 0);
}
