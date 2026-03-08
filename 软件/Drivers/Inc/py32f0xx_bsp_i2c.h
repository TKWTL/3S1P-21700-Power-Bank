/*PY32 基于LL库实现的I2C异步操作库
中断作为基本事件处理框架，使用DMA实现大量数据的收发
目前仅支持7位从机地址的主机模式,不支持总线仲裁*/
#ifndef __PY32F0XX_BSP_I2C_H__
#define __PY32F0XX_BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "coroOS.h"
/*****************************用户配置区开始***********************************/

#define I2C_DMA_BUS                         LL_AHB1_GRP1_PERIPH_DMA1
#define I2C_DMA_CHANNEL_SET_RESPOND_SPEED   LL_SYSCFG_SetDMAResponseSpeed_CH1
#define I2C_DMA_CHANNEL_REMAP               LL_SYSCFG_SetDMARemap_CH1
#define I2C_DMA_CHANNEL                     LL_DMA_CHANNEL_1                    //I2C传输使用的DMA通道
#define I2C_DMA_PRIORITY                    LL_DMA_PRIORITY_MEDIUM              //I2C传输使用的DMA优先级 LOW/MEDIUM/HIGH/VERYHIGH
#define I2C_DMA_IRQn                        DMA1_Channel1_IRQn
#define I2C_DMA_IRQHandler                  DMA1_Channel1_IRQHandler            //DMA1_Channel2_3_IRQHandler
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

#define I2C_LOWEREST_PRIORITY   255U                                            //I2C最低优先级

//I2C系统状态枚举
typedef enum{
    I2C_IDLE = 0x00,        //空闲    
    I2C_TX_ADDR,            //TX模式发送地址
    I2C_TX_ACKED,           //TX模式收到从机应答
    I2C_TX_ING,             //TX模式数据发送中    
    I2C_RX_POINTER_ADDR,    //RX模式写寄存器阶段发送地址
    I2C_RX_POINTER_ACKED,   //RX模式写寄存器阶段收到从机应答
    I2C_RX_POINTER_SENT,    //RX模式发送寄存器
    I2C_RX                  //RX模式接收数据    
}I2C_tranceiver_status;
//I2C DMA状态枚举
typedef enum {
    I2C_DMA_IDLE = 0x00,
    I2C_DMA_BUSY,
    I2C_DMA_DONE
}I2C_DMA_status;

//I2C系统状态结构体定义
struct I2C_StatusTypedef{
    I2C_tranceiver_status i2c1_status;  //I2C外设状态机状态标识
    I2C_DMA_status i2c1_dma_status;     //I2C DMA状态
    uint8_t async_dev_addr;             //设备I2C总线地址
    uint8_t async_reg_addr;             //设备寄存器地址
    uint8_t *async_data;                //指向数据数组首地址
    uint16_t async_len;                 //待收发的数据长度
    uint8_t waiting_priority;           //正在等待的任务优先级，默认为255，此时优先级最低
    uint8_t *async_flag;                //指向状态标志变量
};
/*async_flag，指向用户程序提供的标志位变量，用于提示用户程序数据读取完成/设备无响应
 *0代表处理中
 *1代表处理完成
 *2代表设备无响应
 */
#define I2C_FLAG_PROCESSING         0x00
#define I2C_FLAG_DONE               0x01
#define I2C_FLAG_NORESPONSE         0x02
#define I2C_FLAG_BUSERROR           0x04

extern struct pt_sem i2c_mutex;                                                 //实现互斥访问的信号量

void BSP_I2C_Config(void);                                                      //总线配置
void I2C_Diagnosis(void);//修复总线
ErrorStatus APP_I2C_TestAddress(uint8_t dev_addr);                              //检测总线上特定地址的设备
    
void APP_I2C_Transmit(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t len);//阻塞式发送程序（无异常保护）
void APP_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);//阻塞式接收程序（无异常保护）

I2C_tranceiver_status GetI2CStatus();
/*
*异步I2C发送启动函数
返回值：0：总线忙  1：已启动发送
参数：dev_addr：目标设备地址，8-bit；
      reg_addr：寄存器地址
      *data：发送的数组的首地址
      len：发送长度
      priority：发送优先级，越小优先级越高，低优先级任务在高优先级任务等待时不能触发发送
      *flag：返回反映传输状态的用户变量指针
*/
uint8_t ASYNC_I2C_Transmit(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t priority, uint8_t *flag);
/*
*异步I2C接收启动函数
返回值：0：总线忙  1：已启动接收
参数：dev_addr：目标设备地址，8-bit；
      reg_addr：寄存器地址
      *data：接收目标数组的首地址
      len：接收长度
      priority：接收优先级，越小优先级越高，低优先级任务在高优先级任务等待时不能触发接收
      *flag：返回反映传输状态的用户变量指针
*/
uint8_t ASYNC_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t priority, uint8_t *flag);
    
    
#ifdef __cplusplus
}
#endif

#endif
