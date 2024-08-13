/*
协程式操作系统
*/
#ifndef __COROOS_H__
#define __COROOS_H__

#include "pt.h"                                                                 //基于protothread库实现
#include "pt-sem.h"                                                             //PT库的信号量
//#include//添加操作系统所需数据结构

#include "defines.h"

//以下为使用该系统的一些要求
//定义一个协程函数的方法:static int 函数名(struct pt &pt)
//有一个随时间自增的uint32_t类型变量:millis
extern uint32_t millis;

//在该文件中注册所有的线程
//线程的名字必须满足thread_name()格式，name为线程名
#define OS_REGISTER_N(name) n_thread_##name
#define OS_REGISTER(name) thread_##name (tpt+n_thread_##name);

//在以下两处注册线程的名字（记得加逗号与反斜杠）
#define OS_REGISTER_BLOCK1 \
    OS_REGISTER_N(1),\
    OS_REGISTER_N(I2C_Transmit)
//记得反斜杠
#define OS_REGISTER_BLOCK2 \
    OS_REGISTER(1)\
    OS_REGISTER(I2C_Transmit)
    
//简化编程定义宏
#define OS_REGISTER_BEGIN typedef enum{
    
#define OS_REGISTER_SEG1 \
                            n_thread_num,\
                         }ThreadIndex_t;\
                         struct pt tpt[n_thread_num];\
                         void THRD_INIT(){\
                             uint16_t p_thread;\
                             for(p_thread= 0; p_thread< n_thread_num; p_thread++) PT_INIT(tpt+p_thread);\
                         }\
                         void OS_Run(){while(1){
                             
#define OS_REGISTER_END  }}

/*用法举例：（在main.c文件中，位于所有thread函数的定义与声明下）
OS_REGISTER_BEGIN
OS_REGISTER_BLOCK1
OS_REGISTER_SEG1
OS_REGISTER_BLOCK2
OS_REGISTER_END

int main()
{
    THRD_INIT();
    OS_Run();
    while(1);
}*/
/******************************协程系统定义************************************/
//协程声明
#define THRD_DECLARE(name_args)         char name_args(struct pt *pt)
    
//协程开始
#define THRD_BEGIN                      static uint32_t endmillis;\
                                        PT_BEGIN(pt)

//出让控制权
#define THRD_YIELD                      PT_YIELD(pt)

//系统延时
#define THRD_DELAY(ticks)               endmillis = ticks + millis;\
                                        PT_WAIT_UNTIL(pt,endmillis < millis)
                 
//无超时的条件等待
#define THRD_WHILE(cond)                PT_WAIT_WHILE(pt,cond)
#define THRD_UNTIL(cond)                PT_WAIT_UNTIL(pt,cond)

//带超时处理的条件等待
#define THRD_WHILE_T(cond,ticks,func)   endmillis = ticks + millis;\
                                        if(endmillis < millis) func();\
                                        else PT_WAIT_WHILE(pt,cond)
#define THRD_UNTIL_T(cond,ticks,func)   endmillis = ticks + millis;\
                                        if(endmillis < millis) func();\
                                        else PT_WAIT_UNTIL(pt,cond)
                                            
//协程结束                 
#define THRD_END                        PT_END(pt)
                                        
#endif