#ifndef PY32F003_BSP_GPIO_H
#define PY32F003_BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "coroOS.h"
typedef enum
{
  LED3 = 0,
  LED_GREEN = LED3
} Led_TypeDef;

#define LEDn                               1

#define LED3_PIN                           LL_GPIO_PIN_5
#define LED3_GPIO_PORT                     GPIOB
#define LED3_GPIO_CLK_ENABLE()             LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB)
#define LED3_GPIO_CLK_DISABLE()            LL_IOP_GRP1_DisableClock(LL_IOP_GRP1_PERIPH_GPIOB)

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)    do {LED3_GPIO_CLK_ENABLE(); } while(0U)
#define LEDx_GPIO_CLK_DISABLE(__INDEX__)   LED3_GPIO_CLK_DISABLE()


void             BSP_LED_Init(Led_TypeDef Led);
void             BSP_LED_DeInit(Led_TypeDef Led);
void             BSP_LED_On(Led_TypeDef Led);
void             BSP_LED_Off(Led_TypeDef Led);
void             BSP_LED_Toggle(Led_TypeDef Led);

void BSP_GPIO_Config();

#ifdef __cplusplus
}
#endif

#endif
