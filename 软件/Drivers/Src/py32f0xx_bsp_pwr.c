#include "py32f0xx_bsp_pwr.h"

void BSP_PWR_Config()
{
    LL_LPM_DisableEventOnPend();//仅已使能事件和中断才能唤醒处理器
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
    LL_PWR_EnableLowPowerRunMode();//使用LPR为Stop模式下的芯片供电
    LL_LPM_DisableSleepOnExit();//禁止中断服务程序退出即休眠
    LL_LPM_EnableSleep();//WFI后仅CPU停摆
}
    