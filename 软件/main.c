#include "main.h"
#include "sw6306.h"
#include "debounce_key.h"

uint8_t ledsta = 0;//LED状态全局变量
uint8_t forceoff = 0;//关闭输出用全局变量
uint16_t cd_sleep = SLEEP_DELAY;//睡眠倒计时

extern uint8_t inttrig, keytrig;//bsp_exti.c文件定义的指示变量


THRD_DECLARE(thread_app)
{
    static uint16_t deattach_delay;
    static uint8_t events;//REG0x15事件消费缓存（protothread跨yield变量必须static）
    THRD_BEGIN;
    THRD_SPAWN_NOARG(SW6306_ForceOff);
    THRD_SPAWN_NOARG(SW6306_Init);
    while(1)
    {
        uprintf("\n\nTime before sleep:%d0ms",cd_sleep);   
        
        THRD_SPAWN_NOARG(SW6306_ADCLoad);
        THRD_SPAWN_NOARG(SW6306_PortStatusLoad);
        THRD_DELAY(REFRESH_DELAY/ 4);
        THRD_SPAWN_NOARG(SW6306_StatusLoad);
        THRD_SPAWN_NOARG(SW6306_PowerLoad);
        THRD_DELAY(REFRESH_DELAY/ 4);
        THRD_SPAWN_NOARG(SW6306_CapacityLoad);
        THRD_DELAY(REFRESH_DELAY/ 4);
        
        //消费REG0x15事件：读取→打印原因→W1C清除，防止历史事件被当作实时状态
        events = SW6306_ReadEventFlags();
        if(events & SW6306_FAULT0_MSK)
        {
            if(events & SW6306_FAULT0_UVLO)      uprintf("\nUVLO event occurred.");
            if(events & SW6306_FAULT0_CHGERR)    uprintf("\nCharge fault event occurred.");
            if(events & SW6306_FAULT0_DISCHGERR) uprintf("\nDischarge fault event occurred.");
            if(events & SW6306_FAULT0_KEY)       uprintf("\nKey event occurred.");
            if(events & SW6306_FAULT0_SCENE)     uprintf("\nScene change event occurred.");
            if(events & SW6306_FAULT0_LEARNEND)  uprintf("\nCapacity learning finished.");
            //打印历史故障原因（REG0x2A~0x2C），便于定位具体保护
            if(events & (SW6306_FAULT0_UVLO|SW6306_FAULT0_CHGERR|SW6306_FAULT0_DISCHGERR))
                uprintf("\nEVENT=0x%02X SYS=0x%02X FAULT_D=0x%02X FAULT_C=0x%02X",
                        SW6306_ReadEventFlags(), SW6306_ReadSystemStatus(),
                        SW6306_ReadFaultDischarge(), SW6306_ReadFaultCharge());
            THRD_SPAWN_ARGS(SW6306_ClearEvents, events);
        }
        
        if(SW6306_IsInitialized() == 0)//检测SW6306是否已初始化过
        {
            cd_sleep = SLEEP_DELAY;
            uprintf("\nReInitializing SW6306..."); 
            THRD_SPAWN_NOARG(SW6306_ForceOff);
            THRD_SPAWN_NOARG(SW6306_Init);
        }            
        
        //A口非空状态且BUS电流过小且LED未开启时延时发送A口拔出事件，以保证单口输入输出时的快充
        if((SW6306_ReadIBUS()<IBUS_NOLOAD&&ledsta == 0)&&(SW6306_IsPortA1ON()||SW6306_IsPortA2ON())) deattach_delay++;
        //A口非空状态且充电中、C口非空满足其一时立刻发送A口拔出事件，以保证单口输入输出时的快充
        else if((SW6306_IsCharging()||SW6306_IsPortC1ON()||SW6306_IsPortC2ON())&&(SW6306_IsPortA1ON()||SW6306_IsPortA2ON())) deattach_delay = A_DEATTACH_DELAY;
        else deattach_delay = 0;
        //充电状态、充满状态、BUS与BAT电流足够大时刷新睡眠倒计时
        if((SW6306_ReadIBAT()>IBAT_NOLOAD)||(SW6306_ReadIBUS()>IBUS_NOLOAD)||SW6306_IsPortC1ON()||SW6306_IsPortC2ON()||SW6306_IsPortA1ON()||SW6306_IsPortA2ON()) cd_sleep = SLEEP_DELAY;

        //充放电状态显示（异常用REG0x18实时状态，避免历史事件误报）
        if(SW6306_IsCharging()) uprintf("\nCharging.");
        if(SW6306_IsDischarging()) uprintf("\nDischarging.");
        if(SW6306_HasFullChargeEvent()) uprintf("\nFull Charged.");
        else{
            if(SW6306_IsChargeStoppedByFault()) uprintf("\nCharging stopped by fault.");
            if(SW6306_IsDischargeStoppedByFault()) uprintf("\nDischarging stopped by fault.");
        }
            
        //端口状态显示
        if(SW6306_IsPortC1ON()) uprintf("\nPort C1 Path Enabled.");
        if(SW6306_IsPortC2ON()) uprintf("\nPort C2 Path Enabled.");
        if(SW6306_IsPortA1ON()) uprintf("\nPort A1 Path Enabled.");
        if(SW6306_IsPortA2ON()) uprintf("\nPort A2 Path Enabled.");
        uprintf("\nProtocol: %s", SW6306_ReadProtocol());
        //电气数据显示
        uprintf("\nCapacity:%d%%",SW6306_ReadCapacity());
        uprintf("\nPortBus Voltage:%dmV",SW6306_ReadVBUS());     
        uprintf("\tPortBus Current:%dmA",SW6306_ReadIBUS());
        uprintf("\nBattery Voltage:%dmV",SW6306_ReadVBAT());
        uprintf("\tBattery Current:%dmA",SW6306_ReadIBAT());
        uprintf("\nNTC Temprature:%d'C",SW6306_ReadTNTC());
        uprintf("\t\tChip Temprature:%.2f'C\n",SW6306_ReadTCHIP());
        
        //A口断开操作的执行
        if(deattach_delay >= A_DEATTACH_DELAY)
        {
            THRD_SPAWN_NOARG(SW6306_PortA1Remove);
            THRD_SPAWN_NOARG(SW6306_PortA2Remove);
            deattach_delay = 0;
        }
        
        //LED耗电量计算
        else if(ledsta)
        {
            cd_sleep = SLEEP_DELAY;//刷新睡眠倒计时        
            //没有口打开时触发口插入以持续显示电量和使能电量计算  
            if(!(SW6306_IsPortC1ON()||SW6306_IsPortC2ON()||SW6306_IsPortA1ON()||SW6306_IsPortA2ON()))
            {
                THRD_DELAY(150);//延迟以使C口先打开
                THRD_SPAWN_NOARG(SW6306_PortStatusLoad);
                if(!(SW6306_IsPortC1ON()||SW6306_IsPortC2ON()||SW6306_IsPortA1ON()||SW6306_IsPortA2ON())) THRD_SPAWN_NOARG(SW6306_PortA1Insert);
            }
        }
        
        THRD_DELAY(REFRESH_DELAY/ 4);
    }
    THRD_END;
}

THRD_DECLARE(thread_echo)
{
    uint8_t buf[32];
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

uint16_t wdt_cnt = 0;
THRD_DECLARE(thread_key)
{
    static uint8_t step,cnt;//双击计时用变量
    static uint8_t holding;//按键按住
    static uint8_t leddir = 1;//调光方向
    static uint8_t ledind = 127;//LED亮度
    static uint16_t cd_reset;
    THRD_BEGIN;
    while(1)
    {
        Key_DebounceService_10ms();
        Key_Scand();
        
        if(cnt) cnt--;//计时器自减直到0
        else step = 0;
        
        KeyEdge_t edge = Key_EdgeDetect(KeyIndex_KEY);

        if(edge == KeyEdge_Rising)
        {
            if(step == 0)
            {
                cnt = TMAX_DOUBLECLICK; //首次按下
                step = 1;
                THRD_SPAWN_NOARG(SW6306_Click);
            }
            else if(step == 1)
            {
                cnt = TMAX_DOUBLECLICK; //第二次按下
                step = 2;
            }
            else
            {
                step = 0;
            }
        }
        else if(edge == KeyEdge_Falling)
        {
            if(step == 1)
            {
                cnt = TMAX_DOUBLECLICK; //首次松开
            }
            else if(step == 2) //第二次松开，触发双击
            {
                cnt = 0;
                if(ledsta)
                {
                    uprintf("\n\nWLED Off!\n");
                    ledsta = 0;
                    LED_PWM_Set(0);
                    THRD_DELAY(1);
                    LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
                }
                else
                {
                    uprintf("\n\nWLED On!\n");
                    ledsta = 1;
                    LL_GPIO_SetOutputPin(LED_PORT, LED_PIN);
                    THRD_DELAY(1);
                    LED_PWM_Set(ledind);
                }
            }
            else
            {
                step = 0;
            }
            holding = 0; //松开时解除长按
        }
        else if(edge == KeyEdge_Holding) //触发长按
        {
            holding = 1;
            if(ledsta) //LED打开时长按调光，每次触发长按时改变调光方向
            {
                if(leddir) leddir = 0;
                else       leddir = 1;
            }
            else
            {
                forceoff = 1; //LED关闭时长按关闭输出
            }
        }
        
        if(ledsta && SW6306_IsBatteryLowNow())//实时VBAT低压关闭WLED
        {
            uprintf("\n\nWLED OFF: battery low, VBAT=%dmV\n\n", SW6306_ReadVBAT());
            ledsta = 0;
            LED_PWM_Set(0);
            THRD_DELAY(1);
            LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
        }
        else if(ledsta && SW6306_IsOverheatedNow())//实时过温关闭WLED
        {
            uprintf("\n\nWLED OFF: over temperature, NTC=%dC CHIP=%.2fC\n\n",
                    SW6306_ReadTNTC(), SW6306_ReadTCHIP());
            ledsta = 0;
            LED_PWM_Set(0);
            THRD_DELAY(1);
            LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
        }
        
        if(ledsta && holding)
        {
            if(leddir && ledind < 254) ledind++;//增加亮度
            else if(!leddir && ledind > 1) ledind--;//减小亮度
            LED_PWM_Set(ledind);
        }
        else if(holding)
        {
            cd_reset+=2;
            if(cd_reset > T_ULTRA_LONGPRESS)//触发了超长按
            {
                uprintf("\n\nResetting......\n\n");
                LL_mDelay(100);
                NVIC_SystemReset();
            }
        }
        else cd_reset = 0;
        
        if(cd_sleep) cd_sleep--;//睡眠计时器自减
        
        //I2C看门狗
        wdt_cnt++;
        if(i2c_mutex.count) wdt_cnt = 0;
        else if(wdt_cnt > 100 && GetI2CStatus() == I2C_IDLE){
            uprintf("\n\nI2C WatchDog Triggerd!!!\n\n");
            I2C_Diagnosis();
            PT_SEM_SIGNAL(pt, &i2c_mutex);
        }
        
        THRD_DELAY(1);
    }
    THRD_END;
}

THRD_DECLARE(thread_trig)
{
    THRD_BEGIN;
    while(1)
    {
        if(inttrig)
        {
            uprintf("\n\nIRQ event occured!\n");
            //THRD_SPAWN_NOARG(SW6306_StatusLoad);
            inttrig = 0;
        }
        if(keytrig)
        {
            uprintf("\nKEY Pressed!\n");
            keytrig = 0;
        }
        if(forceoff)
        {
            forceoff = 0;
            THRD_SPAWN_NOARG(SW6306_ForceOff);
        }
        THRD_YIELD;
    }
    THRD_END;
}

THRD_DECLARE(thread_sleep){                                                     //低功耗休眠线程，放在线程函数注册表的末尾
    THRD_BEGIN;
    while(1)
    {   //进入深度睡眠必须满足的条件：
        //按键松开、睡眠倒计时归零、I2C没有正在读写的任务且未进行串口打印
        //满足条件时关闭所有外设：
        //打开SW6306低功耗模式，关闭Systick定时器中断，允许DeepSleep
        if(USART_IsBusy() == 0&&
            cd_sleep == 0&&
            i2c_mutex.count&&
            LL_GPIO_IsInputPinSet(KEY_PORT, KEY_PIN))
        {
            while(SW6306_LPSet(pt)==0);//while代替THRD_DELAY
            LL_mDelay(1);//延时等待操作完成
            LL_SYSTICK_DisableIT();
            LL_LPM_EnableDeepSleep();
            __WFI();
            //睡眠/唤醒分界线
            LL_SYSTICK_EnableIT();
            LL_LPM_EnableSleep();
            while(SW6306_Unlock(pt)==0);
        }
        else __WFI();//否则浅度睡眠
        THRD_YIELD;
    }
    THRD_END;
}


char (*threads[])(struct pt *pt) = {                                            //线程函数指针数组，在这里注册要运行的线程函数名字
    thread_echo,//将串口收到的数据直接发回
    thread_key,//按键、调光与系统状态
    thread_trig,//EXTI响应
    thread_app,//SW6306相关操作
    thread_sleep
};
uint8_t thread_num = sizeof(threads)/sizeof(char(*)(struct pt *pt));            //线程数目指示

int main(void)
{
    SysInit();
    Key_Init();
    LL_mDelay(50);//等待SW6306上电稳定
    
    uprintf("\n\n3S1P 21700 Power Bank");
    uprintf("\nPowered by SW6306 & PY32F002A");
    uprintf("\nTKWTL 2026/08/08\n");
    
    OS_INIT(threads);
    
    while(1)//主循环
    {
        OS_RUN(threads);
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
