#ifndef __PY32F0XX_BSP_TIM_H__
#define __PY32F0XX_BSP_TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "coroOS.h"

void BSP_TIM_config(void);
void BSP_PWMChannelConfig(void);
    
void LED_PWM_Set(uint8_t indensity);
    
#ifdef __cplusplus
}
#endif

#endif
