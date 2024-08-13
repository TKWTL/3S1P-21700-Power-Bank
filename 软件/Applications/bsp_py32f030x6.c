#include "bsp_py32f030x6.h"




void SysInit()
{
    BSP_SystemClockConfig();
    BSP_PWR_Config();
    BSP_GPIO_Config();
    BSP_EXTI_Config();
    BSP_TIM_config();
    BSP_PWMChannelConfig();
    BSP_USART_Config(BAUD_RATE);
    BSP_I2C_Config();
    
}