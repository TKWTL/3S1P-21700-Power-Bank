#include "py32f0xx_bsp_tim.h"

void BSP_TIM_config(void)
{
    LL_TIM_InitTypeDef TIMCountInit = {0};

    LL_APB1_GRP2_EnableClock(LED_PWM_TIM_CLK);
 
    TIMCountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
    TIMCountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;
    TIMCountInit.Prescaler           = 0;
    /* PWM period = 1000 */
    TIMCountInit.Autoreload          = LED_PWM_TOP-1;
    TIMCountInit.RepetitionCounter   = 0;
    LL_TIM_Init(LED_PWM_TIM,&TIMCountInit);

    LL_TIM_EnableAllOutputs(LED_PWM_TIM);
    LL_TIM_EnableCounter(LED_PWM_TIM);
}
    
void BSP_PWMChannelConfig(void)
{
    LL_GPIO_InitTypeDef GPIO_InitTypeDef;
    LL_TIM_OC_InitTypeDef TIM_OC_Initstruct;

    GPIO_InitTypeDef.Pin = LED_PWM_PIN;
    GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitTypeDef.Alternate = LED_PWM_AF;
    GPIO_InitTypeDef.Pull = LL_GPIO_PULL_DOWN;
    LL_GPIO_Init(LED_PWM_PORT, &GPIO_InitTypeDef);

    TIM_OC_Initstruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_Initstruct.OCState = LL_TIM_OCSTATE_ENABLE;
    TIM_OC_Initstruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_Initstruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;

    /* Set channel compare values */
    TIM_OC_Initstruct.CompareValue = 0;
    LL_TIM_OC_Init(LED_PWM_TIM, LL_TIM_CHANNEL_CH1, &TIM_OC_Initstruct);
}

/*LED亮度控制函数
/将入参(1~255)按二次曲线映射到LED_PWM_MIN与LED_PWM_MAX之间，以达到平滑的视觉效果
/入参为0时关闭PWM输出
*/
void LED_PWM_Set(uint8_t indensity)
{
    uint16_t i;
    if(indensity)
    {
        i = (indensity*indensity)>>8;
        i = LED_PWM_MIN + (i*(LED_PWM_MAX - LED_PWM_MIN)>>8);
        LED_PWM_SET(i);
    }
    else LED_PWM_SET(0);
}
