#include "debounce_key.h"

#ifndef DEBOUNCE_TIME_10MS
    #define DEBOUNCE_TIME_10MS              2
#endif

#define DEFAULT_LONGPRESS_TIME_10MS     120

/*******************************用户按键配置区*********************************/

static uint8_t KEY_GetIO(void);

const KeyInfo_t KeyInfo_Array[KeyIndex_Max] = {
    /* {LongPressTime, ZeroIsPress, GetIOFunc(Must not NULL)} 
    LongPressTime：为0时使用默认长按时间*/
    {150, 1, KEY_GetIO},
};
/*******************************用户配置区结束*********************************/

/*********************************功能代码*************************************/
static Key_t KeyValue_Array[KeyIndex_Max];                                      //储存着按键所有状态的数组


void Key_Scand(void) {
    uint8_t i;
    uint8_t IOState;

    for (i = 0; i < KeyIndex_Max; ++i) {                                        //对于for循环i++与++i等价，但后者执行效率高
        IOState = KeyInfo_Array[i].GetIOFunc();
        IOState = (KeyInfo_Array[i].ZeroIsPress ? (!IOState) : (IOState));

        if (KeyValue_Array[i].CurrIOState != IOState) {
            KeyValue_Array[i].CurrIOState = IOState;
            KeyValue_Array[i].DebounceCounter = DEBOUNCE_TIME_10MS;             //键值变化，开启消抖计数器
        }

        //Wait for debounce
        if (KeyValue_Array[i].LastIOState != KeyValue_Array[i].CurrIOState) {
            if (KeyValue_Array[i].DebounceCounter == 0) {
                if ((KeyValue_Array[i].CurrIOState) && (!KeyValue_Array[i].LastIOState)) {  //上次为假，这次为真
                    KeyValue_Array[i].State = KeyState_ShortPress;                          //识别为短按
                    
                    if (KeyInfo_Array[i].LongPressTime) {       //如果对应按键开启了长按
                        KeyValue_Array[i].DebounceCounter = KeyInfo_Array[i].LongPressTime;
                    } 
                    else {
                        KeyValue_Array[i].DebounceCounter = DEFAULT_LONGPRESS_TIME_10MS;
                    }
                } 
                else if ((!KeyValue_Array[i].CurrIOState) && (KeyValue_Array[i].LastIOState)) { //上次为真，这次为假
                    KeyValue_Array[i].State = KeyState_Release;                                 //按键被释放
                } 
                else {
                    KeyValue_Array[i].State = KeyState_None;
                }
                KeyValue_Array[i].LastIOState = KeyValue_Array[i].CurrIOState;
            }
        }

        //Wait for LongPress
        if (KeyValue_Array[i].State == KeyState_ShortPress) {
            if (KeyValue_Array[i].DebounceCounter == 0) {
                KeyValue_Array[i].State = KeyState_LongPress;
            }
        }
    }
}

void Key_DebounceService_10ms(void) {
    uint8_t i;
    for (i = 0; i < KeyIndex_Max; ++i) {
        if (KeyValue_Array[i].DebounceCounter) {
            KeyValue_Array[i].DebounceCounter--;
        }
    }
}

void Key_Init(void) {
    uint8_t i = 0;
    for(i = 0; i < KeyIndex_Max; ++i) {
        KeyValue_Array[i].CurrIOState = 0;
        KeyValue_Array[i].LastIOState = 0;
        KeyValue_Array[i].DebounceCounter = 0;
        KeyValue_Array[i].State = KeyState_None;
        KeyValue_Array[i].LastState = KeyState_None;
    }
}

KeyEdge_t Key_EdgeDetect(KeyIndex_t KeySelector) {
    KeyEdge_t ButtonEdge;
    if(KeyValue_Array[KeySelector].State == KeyValue_Array[KeySelector].LastState) ButtonEdge = KeyEdge_Null;
    else if((KeyValue_Array[KeySelector].State == KeyState_ShortPress)&&((KeyValue_Array[KeySelector].LastState == KeyState_None)||(KeyValue_Array[KeySelector].LastState == KeyState_Release))) ButtonEdge = KeyEdge_Rising;
    else if((KeyValue_Array[KeySelector].State == KeyState_Release)&&((KeyValue_Array[KeySelector].LastState == KeyState_ShortPress)||(KeyValue_Array[KeySelector].LastState == KeyState_LongPress))) ButtonEdge = KeyEdge_Falling;
    else if((KeyValue_Array[KeySelector].State == KeyState_LongPress)&&(KeyValue_Array[KeySelector].LastState == KeyState_ShortPress)) ButtonEdge = KeyEdge_Holding;
    else ButtonEdge = KeyEdge_Error;

    KeyValue_Array[KeySelector].LastState = KeyValue_Array[KeySelector].State;
    return ButtonEdge;
}

/*****************************用户配置区开始***********************************/
static uint8_t KEY_GetIO(void) {
    return LL_GPIO_IsInputPinSet(KEY_PORT, KEY_PIN);
}

KeyState_t KEY_GetState(void) {
    return KeyValue_Array[KeyIndex_KEY].State;
}
/*******************************用户配置区结束*********************************/

/*用例：
*
*/