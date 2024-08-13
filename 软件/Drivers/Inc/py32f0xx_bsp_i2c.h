/*PY32 基于LL库实现的I2C异步操作库
目前仅支持7位从机地址的主机模式,不支持总线仲裁*/
#ifndef __PY32F0XX_BSP_I2C_H__
#define __PY32F0XX_BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "coroOS.h"
/*****************************用户配置区开始***********************************/
//I2C是否启用超时检测与实现方式
//#define I2C_WDT_HWTIMER                                                       //使用定时器
#define I2C_WDT_THREAD                                                      //使用线程
    
#define I2C_TIMEOUT_TIME 20
//I2C的标志位检测方法，while(condition)为阻塞调用
#define THRD_I2C_WHILE(condition)     THRD_WHILE_T(condition,10,THRD_I2C_Diagnosis)
    
//I2C的四种使用方式：阻塞等待标志位，线程挂起，中断，DMA
//其中阻塞式默认支持，其他三种需要定义对应宏，只能三选一，不需要的请注释
//#define I2C_USE_THREAD
#define I2C_USE_IT
//#define I2C_USE_DMA//未完成

/*******************************用户配置区结束*********************************/
#ifndef SDA1_PIN
#define SDA1_PIN LL_GPIO_PIN_0                  
#endif
#ifndef SDA1_PORT
#define SDA1_PORT GPIOF                                                         //SDA默认PF0                  
#endif
#ifndef SCL1_PIN
#define SCL1_PIN LL_GPIO_PIN_1                  
#endif
#ifndef SCL1_PORT
#define SCL1_PORT GPIOF                                                         //SCL默认PF1                  
#endif
#ifndef I2C_ADDRESS
#define I2C_ADDRESS        0xAA                                                 //默认本机地址定义
#endif

//I2C系统状态枚举
typedef enum
{
    I2C_IDLE = 0x00,
    
    I2C_TX_ADDR,
    I2C_TX_ACKED,    
    I2C_TX_ING,
    
    I2C_RX_POINTER_ADDR,
    I2C_RX_POINTER_ACKED,
    I2C_RX_POINTER_SENT,
    I2C_RX
    
}I2C_tranceiver_status;

//I2C系统状态结构体定义
struct I2C_StatusTypedef
{
    uint8_t async_dev_addr;
    uint8_t async_reg_addr;
    uint8_t *async_data;
    uint8_t *async_flag;
    uint16_t async_len;
    I2C_tranceiver_status i2c1_status;
};
/*async_flag，指向用户程序提供的标志位变量，用于提示用户程序数据读取完成/设备无响应
 *0代表处理中
 *1代表处理完成
 *128代表设备无响应
 */
#define I2C_FLAG_PROCESSING         0x00
#define I2C_FLAG_DONE               0x01
#define I2C_FLAG_NORESPONSE         0x02
#define I2C_FLAG_BUSERROR           0x04

THRD_DECLARE(thread_i2c_wdt);

void BSP_I2C_Config(void);                                                      //总线配置
ErrorStatus APP_I2C_TestAddress(uint8_t dev_addr);                              //检测总线上特定地址的设备
    
void APP_I2C_Transmit(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t len);//阻塞式发送程序（无异常保护）
void APP_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);//阻塞式接收程序（无异常保护）

uint8_t ASYNC_I2C_Transmit(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t *flag);
uint8_t ASYNC_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t *flag);//
    
    
#ifdef __cplusplus
}
#endif

#endif
