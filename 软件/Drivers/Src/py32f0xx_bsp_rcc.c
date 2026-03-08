#include "py32f0xx_bsp_rcc.h"

void BSP_SystemClockConfig(void)
{
    /* Enable SYSCFG and PWR clocks */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    
    /* Enable HSI */
    LL_RCC_HSI_Enable();
    LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
    while(LL_RCC_HSI_IsReady() != 1);

    /* Set AHB prescaler */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    /* Configure HSISYS as system clock source */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS);

    /* Set APB1 prescaler*/
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    
    LL_InitTick(CoreClk, 1000U / T_SYSTICK);
    LL_SYSTICK_EnableIT();

    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    
    /* Update system clock global variable SystemCoreClock (can also be updated by calling SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(CoreClk);
    
    //关闭不必要的时钟
    LL_RCC_LSE_Disable();
    LL_RCC_HSE_Disable();
    LL_RCC_LSI_Disable();
}
