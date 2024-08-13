#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__

#define BAUD_RATE           115200                                              //串口通信波特率

#define T_SYSTICK           10                                                  //Systick溢出时间（单位：ms）

#define IBUS_NOLOAD         50                                                  //BUS空载判定电流（单位：mA）
#define IBAT_NOLOAD         50                                                  //BAT空载判定电流（单位：mA）

#define TMAX_DOUBLECLICK    10                                                  //双击超时时间（单位：10ms）
#define REFRESH_DELAY       100                                                 //刷新时间（单位：10ms）
#define SLEEP_DELAY         600                                                 //无事件后休眠时间（单位：10ms）
#define T_ULTRA_LONGPRESS   1000                                                //超长按响应时间（单位：10ms）

#define LED_PWM_TOP         1024                                                //LED PWM计数最大值/周期
#define LED_PWM_MIN         4                                                   //LED启动时的最小PWM值，决定了最小亮度
#define LED_PWM_MAX         768                                                 //LED启动时的最大PWM值，决定了最大亮度

#endif