/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sw6306.h"
#include "debounce_key.h"
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

uint8_t sendbuff;

uint8_t forceoff;//关闭输出用全局变量
uint8_t main_looping;
uint16_t cd_sleep;//睡眠倒计时

extern uint8_t inttrig,keytrig;
THRD_DECLARE(thread_app)
{
    THRD_BEGIN;
    THRD_UNTIL(SW6306_Init());
    while(1)
    {
        main_looping = 1;//刷新循环开始，此时不可休眠
        
        //millis过大时复位整个系统，避免因溢出产生不可预计的后果，预计溢出时间：497天
        if(millis > 4294960000U) NVIC_SystemReset();
        
        uprintf("\n\nTime before sleep:%d0ms",cd_sleep);        
        
        THRD_UNTIL(SW6306_ADCLoad());
        THRD_UNTIL(SW6306_PortStatusLoad());
        THRD_UNTIL(SW6306_StatusLoad());
        THRD_UNTIL(SW6306_PowerLoad());
        THRD_UNTIL(SW6306_CapacityLoad());
        
        if(SW6306_IsInitialized()==0)//检测SW6306是否已初始化过
        {
            uprintf("\n\nReInitializing SW6306..."); 
            THRD_UNTIL(SW6306_Init());
        }
        
        //A口非空状态且充电中或BUS电流过小满足其一时持续发送A口拔出事件，以保证单口输入输出时的快充
        if((SW6306_IsCharging()||(SW6306_ReadIBUS() <= IBUS_NOLOAD))&&(SW6306_IsPortA1ON()||SW6306_IsPortA2ON())) THRD_UNTIL(SW6306_ByteWrite(SW6306_CTRG_PORTEVT, 0x0A));
        //非充电状态下当BUS与BAT电流至少其一足够大时持续发送短按键事件，以保持显示常亮
        if((SW6306_ReadIBAT() > IBAT_NOLOAD || SW6306_ReadIBUS() > IBUS_NOLOAD) && !(SW6306_IsCharging())) THRD_UNTIL(SW6306_Click());
        //充电状态、充满状态、BUS与BAT电流足够大时刷新睡眠倒计时
        if((SW6306_ReadIBAT() > IBAT_NOLOAD)||(SW6306_ReadIBUS() > IBUS_NOLOAD)||SW6306_IsPortC1ON()||SW6306_IsPortC2ON()) cd_sleep = SLEEP_DELAY;
        
        if(SW6306_IsCharging()) uprintf("\nCharging.");
        if(SW6306_IsDischarging()) uprintf("\nDischarging.");
        if(SW6306_IsFullCharged()) uprintf("\nFull Charged.");
        
        if(SW6306_IsPortC1ON()) uprintf("\nPort C1 Path Enabled.");
        if(SW6306_IsPortC2ON()) uprintf("\nPort C2 Path Enabled.");
        if(SW6306_IsPortA1ON()) uprintf("\nPort A1 Path Enabled.");
        if(SW6306_IsPortA2ON()) uprintf("\nPort A2 Path Enabled.");
        
        uprintf("\nCapacity:%d%%",SW6306_ReadCapacity());
        uprintf("\nPortBus Voltage:%dmV",SW6306_ReadVBUS());     
        uprintf("\nPortBus Current:%dmA",SW6306_ReadIBUS());
        uprintf("\nBattery Voltage:%dmV",SW6306_ReadVBAT());
        uprintf("\nBattery Current:%dmA",SW6306_ReadIBAT());
        uprintf("\nNTC Temprature:%d°C",SW6306_ReadTNTC());
        uprintf("\nChip Temprature:%.2f°C\n",SW6306_ReadTCHIP());
        
        if(forceoff)
        {
            forceoff = 0;
            THRD_UNTIL(SW6306_ForceOff());
        }
        if(inttrig==2)
        {
            THRD_UNTIL(SW6306_StatusLoad());
            inttrig = 0;
        }
        if(keytrig==2)
        {
            THRD_UNTIL(SW6306_Click());
            keytrig = 0;
        }
        
        THRD_DELAY(2);//等待打印完成
        main_looping = 0;//循环告一段落，可以进Stop了
        THRD_DELAY(REFRESH_DELAY-3);
    }
    THRD_END;
}

THRD_DECLARE(thread_echo)
{
    char buf[32];
    uint8_t num;
    THRD_BEGIN;
    while(1)
    {
        THRD_UNTIL(USART_getvalidnum());
        num = USART_getvalidnum();
        USART_bufread(buf,num);
        USART_bufsend(buf,num);
    }
    THRD_END;
}

THRD_DECLARE(thread_key)
{
    static uint8_t step,cnt;//双击计时用变量
    static uint8_t holding,ledsta,leddir,ledind;//按键按住、LED是否打开、调光方向与LED亮度
    static uint16_t cd_reset;
    THRD_BEGIN;
    while(1)
    {
        Key_DebounceService_10ms();
        Key_Scand();
        
        if(cnt) cnt--;//计时器自减直到0
        else step = 0;
        
        switch(Key_EdgeDetect(KeyIndex_KEY))
        {
            case KeyEdge_Rising:
                if(step == 0)
                {
                    cnt = TMAX_DOUBLECLICK;//首次按下
                    step = 1;
                }
                else if(step == 1)
                {
                    cnt = TMAX_DOUBLECLICK;//第二次按下
                    step = 2;
                }
                else step = 0;
                break;
            case KeyEdge_Falling:
                if(step == 1) cnt = TMAX_DOUBLECLICK;//首次松开
                else if(step == 2)//第二次松开，触发双击
                {
                    cnt = 0;
                    if(ledsta)
                    {
                        uprintf("\n\nWLED Off!\n");
                        ledsta = 0;
                        LED_PWM_Set(0);
                        LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
                    }
                    else
                    {
                        uprintf("\n\nWLED On!\n");
                        ledsta = 1;
                        LL_GPIO_SetOutputPin(LED_PORT,LED_PIN);
                        LED_PWM_Set(ledind+1);
                    }
                }
                else step = 0;
                holding = 0;//松开时解除长按
                break;
            case KeyEdge_Holding://触发长按
                holding = 1;
                if(ledsta)//LED打开时长按调光，每次触发长按时改变调光方向
                {
                    if(leddir) leddir = 0;
                    else leddir = 1;
                }
                else forceoff = 1;//LED关闭时长按关闭输出
                break;
            case KeyEdge_Error:
            case KeyEdge_Null:
            default: break;
        }
        
        if(ledsta && (SW6306_IsBatteryDepleted()||SW6306_IsOverHeated()))//低电压与过温关闭WLED
        {
            uprintf("\n\nBattery Depleted!Unable to Enable WLED\n\n");
            ledsta = 0;
            LED_PWM_Set(0);
            LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
        }
        
        if(ledsta && holding)
        {
            if(leddir && (ledind < 254)) ledind++;//增加亮度
            else if(!leddir && ledind) ledind--;//减小亮度
            LED_PWM_Set(ledind+1);
        }
        else if(holding)
        {
            cd_reset++;
            if(cd_reset > T_ULTRA_LONGPRESS)//触发了超长按
            {
                uprintf("\n\nResetting......\n\n");
                LL_mDelay(100);
                NVIC_SystemReset();
            }
        }
        else cd_reset = 0;
        
        if(cd_sleep) cd_sleep--;//睡眠计时器自减
        THRD_DELAY(1);
    }
    THRD_END;
}

THRD_DECLARE(thread_trig)
{
    THRD_BEGIN;
    while(1)
    {
        if(inttrig==1)
        {
            uprintf("\n\nIRQ event occured!\n");
            inttrig = 2;
        }
        if(keytrig==1)
        {
            //uprintf("\nKEY Pressed!\n");
            keytrig = 2;
        }
        THRD_YIELD;
    }
    THRD_END;
}

struct pt pt1,pt2,pt3,pt4;//ProtoThread库运行变量

int main(void)
{
    SysInit();
    Key_Init();
    cd_sleep = SLEEP_DELAY;//刷新睡眠倒计时
    LL_mDelay(50);//等待SW6306上电稳定
    
    uprintf("\n\n3S1P 21700 Power Bank");
    uprintf("\nPowered by SW6306 & PY32F002A");
    uprintf("\nTKWTL 2024/08/10\n");
    
    PT_INIT(&pt1);
    PT_INIT(&pt2);
    PT_INIT(&pt3);
    PT_INIT(&pt4);
    
    while(1)//主循环
    {
        thread_app(&pt1);//SW6306相关操作
        thread_echo(&pt2);//将串口收到的数据直接发回
        thread_key(&pt3);//按键、调光与系统状态
        thread_trig(&pt4);//EXTI响应
        
        //进入深度睡眠必须满足的条件：
        //按键松开、睡眠倒计时归零、I2C没有正在读写的任务且未进行串口打印
        //满足条件时关闭所有外设：
        //打开SW6306低功耗模式，关闭Systick定时器中断，允许DeepSleep
        if((main_looping==0)&&(cd_sleep==0)&&(LL_GPIO_IsInputPinSet(KEY_PORT, KEY_PIN)))
        {
            while(SW6306_LPSet()==0);//while代替THRD_DELAY
            LL_mDelay(1);//延时等待操作完成
            LL_SYSTICK_DisableIT();
            LL_LPM_EnableDeepSleep();
            __WFI();
            LL_SYSTICK_EnableIT();
            LL_LPM_EnableSleep();
            while(SW6306_Unlock()==0);
        }
        else __WFI();//否则浅度睡眠
    }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
    /* Infinite loop */
    while (1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */
