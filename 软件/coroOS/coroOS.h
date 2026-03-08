/*
 *协程式操作系统
 *基于protothread库
 *基本思想为：函数可以保存现有状态退出，并在下次调用时回到原位
 *            在不满足设定条件时主动跳出函数（出让CPU）以让其它任务执行
 *            例如用THRD_DELAY代替delay，以在延时期间做点其他的事
 *
 *必须有一个随时间自增的uint32_t类型变量:millis
 */
#ifndef __COROOS_H__
#define __COROOS_H__

#include "pt.h"                                                                 //基于protothread库实现
#include "pt-sem.h"                                                             //PT库的信号量
#include "stdint.h"                                                             //添加操作系统所需数据结构

#include "defines.h"


extern volatile uint32_t millis;



/*用法举例：（在main.c文件中，位于所有thread函数的定义与声明下）*/
//定义一个协程函数的方法:char 函数名(struct pt &pt)

/******************************协程系统定义************************************/
//协程声明
#define THRD_DECLARE(name_args)         char name_args(struct pt *pt)
    
//协程开始
#define THRD_BEGIN                      static uint32_t endmillis;\
                                        static struct pt subpt;\
                                        PT_BEGIN(pt)

//出让控制权
#define THRD_YIELD                      PT_YIELD(pt)

//系统延时
#define THRD_DELAY(ticks)               \
                                        do{\
                                            endmillis = ticks + millis - 1;\
                                            PT_WAIT_UNTIL(pt,endmillis < millis);\
                                        }while(0)
                 
//无超时的条件等待
#define THRD_WHILE(cond)                PT_WAIT_WHILE(pt,cond)
#define THRD_UNTIL(cond)                PT_WAIT_UNTIL(pt,cond)

//带超时处理的条件等待
#define THRD_WHILE_T(cond,ticks,func)   \
                                        do{\
                                            endmillis = ticks + millis - 1;\
                                            if(endmillis < millis) func();\
                                            else PT_WAIT_WHILE(pt,cond);\
                                        } while(0)
                                            
#define THRD_UNTIL_T(cond,ticks,func)   \
                                        do{\
                                            endmillis = ticks + millis;\
                                            if(endmillis < millis) func();\
                                            else PT_WAIT_UNTIL(pt,cond);\
                                        } while(0)
        
//调用子协程
#define THRD_SPAWN_NOARG(func)          PT_SPAWN(pt, &subpt, func(&subpt))
#define THRD_SPAWN_ARGS(func,...)       PT_SPAWN(pt, &subpt, func(&subpt, __VA_ARGS__))
                                            
//协程结束                 
#define THRD_END                        PT_END(pt)
      
//pt变量初始化，放在main()函数中执行一次
#define OS_INIT(name)                   \
        uint8_t thread_index;\
        struct pt pt_arr[sizeof(name)/sizeof(char(*)(struct pt *pt))];\
        for(thread_index = 0;thread_index < thread_num;thread_index++) PT_INIT(pt_arr + thread_index)
        
//系统主循环，放在main()函数的while(1)中
#define OS_RUN(name)                    \
        do{\
            for(thread_index = 0;thread_index < thread_num;thread_index++){\
                (*name[thread_index])(pt_arr + thread_index);\
            }\
        }while(0)
#endif