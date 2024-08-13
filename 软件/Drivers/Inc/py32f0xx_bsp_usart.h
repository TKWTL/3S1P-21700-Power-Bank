#ifndef __PY32F0XX_BSP_USART_H__
#define __PY32F0XX_BSP_USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "defines.h"
#include "coroOS.h"
#include <stdio.h>
#include <stdarg.h>

/*****************************用户配置区开始***********************************/
//定义该宏，使该库在中断模式下异步运行
#define USART_ENABLEIT
 //定义串口中断收发缓冲区大小   
#define USART_TX_BUFFER_SIZE 256
#define USART_RX_BUFFER_SIZE 32
    
//选择对应的USART与其时钟
#define DEBUG_USART                             USART1
#define DEBUG_USART_CLK_ENABLE()                LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1)
//选择对应的USART的中断向量及中断服务函数名
#define DEBUG_USART_IRQHandler                  USART1_IRQHandler
#define DEBUG_USART_IRQ                         USART1_IRQn

/*******************************用户配置区结束*********************************/
//默认USART引脚:RX默认PA3，TX默认PA2
#ifndef DEBUG_USART_RX_GPIO_PORT
    #define DEBUG_USART_RX_GPIO_PORT                GPIOA
#endif
#ifndef DEBUG_USART_RX_GPIO_CLK_ENABLE
    #define DEBUG_USART_RX_GPIO_CLK_ENABLE()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#endif
#ifndef DEBUG_USART_RX_PIN
    #define DEBUG_USART_RX_PIN                      LL_GPIO_PIN_3
#endif
#ifndef DEBUG_USART_RX_AF
    #define DEBUG_USART_RX_AF                       LL_GPIO_AF_1
#endif
#ifndef DEBUG_USART_TX_GPIO_PORT
    #define DEBUG_USART_TX_GPIO_PORT                GPIOA
#endif
#ifndef DEBUG_USART_TX_GPIO_CLK_ENABLE
    #define DEBUG_USART_TX_GPIO_CLK_ENABLE()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#endif
#ifndef DEBUG_USART_TX_PIN
    #define DEBUG_USART_TX_PIN                      LL_GPIO_PIN_2
#endif
#ifndef DEBUG_USART_TX_AF
    #define DEBUG_USART_TX_AF                       LL_GPIO_AF_1
#endif
//函数声明区
void            BSP_USART_Config(uint32_t baudRate);

void            BSP_UART_TxChar(char ch);
void            BSP_UART_TxHex8(uint8_t hex);
void            BSP_UART_TxHex16(uint16_t hex);
void            BSP_UART_TxHex32(uint32_t hex);
void            BSP_UART_TxString(char *str);


#ifdef USART_ENABLEIT

#if DEBUG_USART_TX_BUFFER_SIZE <= 256
uint8_t         USART_bufsend(uint8_t *UT_Data,uint8_t UT_len);
#else
uint8_t         USART_bufsend(uint8_t *UT_Data,uint16_t UT_len);
#endif

uint8_t         USART_send_a_byte(uint8_t UT_Data);

#if DEBUG_USART_RX_BUFFER_SIZE <= 256
uint8_t         USART_getvalidnum();
#else
uint16_t        USART_getvalidnum();
#endif

uint8_t         USART_read_a_byte();

#if DEBUG_USART_RX_BUFFER_SIZE <= 256
uint8_t         USART_bufread(uint8_t *UT_Data,uint8_t UT_len);
#else
uint8_t         USART_bufread(uint8_t *UT_Data,uint16_t UT_len);
#endif

#endif

void            uprintf(const char* format,...);

#ifdef __cplusplus
}
#endif

#endif
