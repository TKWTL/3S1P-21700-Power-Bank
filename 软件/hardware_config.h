#ifndef __HARDWARE_CONFIG_H__
#define __HARDWARE_CONFIG_H__

/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_ll_rcc.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_system.h"
#include "py32f0xx_ll_cortex.h"
#include "py32f0xx_ll_utils.h"
#include "py32f0xx_ll_pwr.h"
#include "py32f0xx_ll_exti.h"
#include "py32f0xx_ll_adc.h"
#include "py32f0xx_ll_dma.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_i2c.h"
#include "py32f0xx_ll_usart.h"
#include "py32f0xx_ll_tim.h"

/*****************************用户配置区开始***********************************/
#define CoreClk     24000000U

#define SDA1_PIN                                LL_GPIO_PIN_0
#define SDA1_PORT                               GPIOF
#define SCL1_PIN                                LL_GPIO_PIN_1
#define SCL1_PORT                               GPIOF

#define DEBUG_USART_RX_GPIO_PORT                GPIOA
#define DEBUG_USART_RX_GPIO_CLK_ENABLE()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define DEBUG_USART_RX_PIN                      LL_GPIO_PIN_3
#define DEBUG_USART_RX_AF                       LL_GPIO_AF_1

#define DEBUG_USART_TX_GPIO_PORT                GPIOA
#define DEBUG_USART_TX_GPIO_CLK_ENABLE()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define DEBUG_USART_TX_PIN                      LL_GPIO_PIN_2
#define DEBUG_USART_TX_AF                       LL_GPIO_AF_1


#define INT_PIN                                 LL_GPIO_PIN_6
#define INT_PORT                                GPIOA
#define INT_EXTI                                LL_EXTI_LINE_6
#define INT_EXTIPIN                             LL_EXTI_CONFIG_LINE6
#define INT_EXTIPORT                            LL_EXTI_CONFIG_PORTA
#define INT_IRQn                                EXTI4_15_IRQn

#define KEY_PIN                                 LL_GPIO_PIN_3
#define KEY_PORT                                GPIOB
#define KEY_EXTI                                LL_EXTI_LINE_3
#define KEY_EXTIPIN                             LL_EXTI_CONFIG_LINE3
#define KEY_EXTIPORT                            LL_EXTI_CONFIG_PORTB
#define KEY_IRQn                                EXTI2_3_IRQn

#define LED_PIN                                 LL_GPIO_PIN_2
#define LED_PORT                                GPIOB
#define LED_INITSTA                             LL_GPIO_ResetOutputPin

#define LED_PWM_TIM                             TIM14
#define LED_PWM_TIM_CLK                         LL_APB1_GRP2_PERIPH_TIM14
#define LED_PWM_CH                              LL_TIM_CHANNEL_CH1
#define LED_PWM_SET(val)                        LL_TIM_OC_SetCompareCH1(LED_PWM_TIM,val)
#define LED_PWM_PIN                             LL_GPIO_PIN_1
#define LED_PWM_PORT                            GPIOB
#define LED_PWM_AF                              LL_GPIO_AF_0
/*****************************用户配置区结束***********************************/

#endif