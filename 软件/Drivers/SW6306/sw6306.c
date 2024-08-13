#include "sw6306.h"


volatile static struct SW6306_StatusTypedef SW6306_Status;//SW6306状态全局变量

uint8_t SW6306_ByteWrite(uint8_t reg, uint8_t data)
{
    if(SW6306_I2C_Transmit(SW6306_I2C_ADDR, reg, SW6306_Status.sendbuf, 1, &(SW6306_Status.flag)))
    {
        SW6306_Status.sendbuf[0] = data;//可以一会儿再修改发送的值
        return 1;
    }
    else return 0;
}

uint8_t SW6306_ByteRead(uint8_t reg, uint8_t *data)
{
    static uint8_t swrd_steps;
    switch(swrd_steps)
    {
        case 0:
            if(SW6306_I2C_Receive(SW6306_I2C_ADDR, reg, data, 1, &(SW6306_Status.flag))) swrd_steps++;
            else return 0;
        case 1:
            if(SW6306_Status.flag)
            {
                swrd_steps = 0;
                return 1;
            }
            else return 0;
        default:
            swrd_steps = 0;
            return 0;
    }
}
                
/*SW6306寄存器组切换
/通过读取版本寄存器(0x01)的值来确定目前采用的寄存器组，并进行切换
/读出0x01时为0x00~0xFF，其他值则为0x100~0x1FF
*/
uint8_t SW6306_RegsetSwitch(uint16_t regset)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_ByteRead(SW6306_STRG_REV,SW6306_Status.sendbuf)) steps++;
            else return 0;
        case 1:
            if(regset > 0xFFU)
            {
                if(SW6306_Status.sendbuf[0] == 0x01)//需要读写高位地址但目前是低地址
                {
                    if(SW6306_ByteWrite(SW6306_CTRG_WREN,0x81))
                    {
                        steps = 0;
                        return 1;
                    }
                    else return 0;
                }
                else//需要读写高位地址且已经是高地址
                {
                    steps = 0;
                    return 1;
                }
            }
            else
            {
                if(SW6306_Status.sendbuf[0] == 0x01)//需要读写低位地址且已经是低地址
                {
                    steps = 0;
                    return 1;
                }
                else//需要读写低位地址但目前是高地址
                {
                    if(SW6306_ByteWrite(SW6306_CTRG_LREGSET,0x00))
                    {
                        steps = 0;
                        return 1;
                    }
                    else return 0;
                }
            }
        default:
            steps = 0;
            return 0;
    }
}

/*SW6306修改对应寄存器的特定位，不对寄存器组作检查
/遵循读-修改-写的顺序
/将mask中为1的位按data中的对应位修改成1或0，mask中为0的位不被修改
*/
uint8_t SW6306_ByteModify(uint8_t reg, uint8_t mask, uint8_t data)
{
    static uint8_t swmd_steps;
    switch(swmd_steps)
    {
        case 0:
            if(SW6306_ByteRead(reg,SW6306_Status.sendbuf)) 
            {
                SW6306_Status.sendbuf[0] = (SW6306_Status.sendbuf[0] &(~mask)) | (data & mask);
                swmd_steps++;
            }
            else return 0;
        case 1:
            if(SW6306_ByteWrite(reg,SW6306_Status.sendbuf[0]))
            {
                swmd_steps = 0;
                return 1;
            }
            else return 0;
        default:
            swmd_steps = 0;
            return 0;
    }
}

uint8_t SW6306_ADCRead(uint8_t ch, uint16_t *pData)
{
    static uint8_t sw_steps;
    switch(sw_steps)
    {
        case 0:
            if(SW6306_ByteWrite(SW6306_CTRG_ADC_SET,ch)) sw_steps++;
            else return 0;
        case 1:
            if(SW6306_I2C_Receive(SW6306_I2C_ADDR, SW6306_STRG_ADCL, pData, 2, &(SW6306_Status.flag))) sw_steps++;
            else return 0;
        case 2:
            if(SW6306_Status.flag)
            {
                sw_steps = 0;
                return 1;
            }
            else return 0;
        default:
            sw_steps = 0;
            return 0;
    }
}

uint8_t SW6306_ADCLoad(void)
{
    static uint8_t swld_steps;
    switch(swld_steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_CTRG_ADC_SET)) swld_steps++;
            else return 0;
        case 1:
            if(SW6306_ADCRead(SW6306_ADC_SET_VBUS, &SW6306_Status.vbus))
            {
                swld_steps++;
                SW6306_Status.vbus = SW6306_Status.vbus<<3;                     //转换BUS电压
            }
            else return 0;
        case 2:
            if(SW6306_ADCRead(SW6306_ADC_SET_IBUS, &SW6306_Status.ibus))
            {
                swld_steps++;
                SW6306_Status.ibus = SW6306_Status.ibus<<2;                     //转换BUS电流
            }
            else return 0;
        case 3:
            if(SW6306_ADCRead(SW6306_ADC_SET_VBAT, &SW6306_Status.vbat))
            {
                swld_steps++;
                SW6306_Status.vbat = SW6306_Status.vbat*7;                      //转换BAT电压
            }
            else return 0;
        case 4:
            if(SW6306_ADCRead(SW6306_ADC_SET_IBAT, &SW6306_Status.ibat))
            {
                swld_steps++;
                SW6306_Status.ibat = SW6306_Status.ibat*5;                      //转换BAT电流
            }
            else return 0;
        case 5:
            if(SW6306_ADCRead(SW6306_ADC_SET_TNTC, &SW6306_Status.tntc)) swld_steps++;
            else return 0;
        case 6:
            if(SW6306_ADCRead(SW6306_ADC_SET_TCHIP, &SW6306_Status.tchip)) swld_steps++;
            else return 0;
        case 7:
            if(SW6306_ADCRead(SW6306_ADC_SET_VNTC, &SW6306_Status.vntc)) swld_steps++;
            else return 0;
        case 8:
            if(SW6306_Status.tchip <= 3203U)
            {
                swld_steps = 0;
                return 1;
            }
            else swld_steps++;//如果读到错误数据
        case 9:
            if(SW6306_Init())
            {
                swld_steps = 0;
                return 1;
            }
            else return 0;
        default:
            swld_steps = 0;
            return 0;
    }
}

uint16_t SW6306_ReadVBUS(void)//读取BUS电压
{
    return SW6306_Status.vbus;
}
uint16_t SW6306_ReadIBUS(void)//读取BUS电流
{
    return SW6306_Status.ibus;
}
uint16_t SW6306_ReadVBAT(void)//读取BAT电压
{
    return SW6306_Status.vbat;
}
uint16_t SW6306_ReadIBAT(void)//读取BAT电流
{
    return SW6306_Status.ibat;
}
int16_t SW6306_ReadTNTC(void)//读取并转换NTC温度
{
    return (SW6306_Status.tntc-16)*5;
}
float SW6306_ReadTCHIP(void)//读取并转换芯片温度
{
    return (float)(SW6306_Status.tchip-1839)/6.82;
}
float SW6306_ReadVNTC(void)//读取并转换NTC电压
{
    return (float)SW6306_Status.vntc*1.1;
}

uint8_t SW6306_PortStatusLoad(void)//更新端口状态镜像寄存器(0x13,0x18,0x19,0x1C,0x1D)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_STRG_NOLOAD)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteRead(SW6306_STRG_NOLOAD, &SW6306_Status.noload)) steps++;//0x13
            else return 0;
        case 2:
            if(SW6306_ByteRead(SW6306_STRG_SYS_STAT, &SW6306_Status.sys_stat)) steps++;//0x18
            else return 0;
        case 3:
            if(SW6306_ByteRead(SW6306_STRG_TYPEC, &SW6306_Status.typec_stat)) steps++;//0x19
            else return 0;
        case 4:
            if(SW6306_ByteRead(SW6306_STRG_TYPEA_QCIN, &SW6306_Status.typea_qcin)) steps++;//0x1C
            else return 0;
        case 5:
            if(SW6306_ByteRead(SW6306_STRG_PORT_STA, &SW6306_Status.port_stat))//0x1D
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}
uint8_t SW6306_IsPortC1ON(void)
{
    return SW6306_Status.sys_stat & SW6306_SYS_STAT_C1ON;
}
uint8_t SW6306_IsPortC2ON(void)
{
    return SW6306_Status.sys_stat & SW6306_SYS_STAT_C2ON;
}
uint8_t SW6306_IsPortA1ON(void)
{
    return SW6306_Status.sys_stat & SW6306_SYS_STAT_A1ON;
}
uint8_t SW6306_IsPortA2ON(void)
{
    return SW6306_Status.sys_stat & SW6306_SYS_STAT_A2ON;
}


uint8_t SW6306_PowerLoad(void)//更新功率状态镜像寄存器(0x0E,0x0F,0x10,0x11,0x1C,0x45,0x4F,0x51,0x52)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_STRG_VBUS_CHG)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteRead(SW6306_STRG_VBUS_CHG, &SW6306_Status.vbus_chg)) steps++;//0x0E
            else return 0;
        case 2:
            if(SW6306_ByteRead(SW6306_STRG_QCSTAT, &SW6306_Status.qcstat)) steps++;//0x0F
            else return 0;
        case 3:
            if(SW6306_ByteRead(SW6306_STRG_BUSILIM_CHG, &SW6306_Status.ibuslim_chg)) steps++;//0x10
            else return 0;
        case 4:
            if(SW6306_ByteRead(SW6306_STRG_BATILIM_CHG, &SW6306_Status.ibatlim_chg)) steps++;//0x11
            else return 0;
        case 5:
            if(SW6306_ByteRead(SW6306_STRG_TYPEA_QCIN, &SW6306_Status.typea_qcin)) steps++;//0x1C
            else return 0;
        case 6:
            if(SW6306_ByteRead(SW6306_CTRG_PISET, &SW6306_Status.pimax_set)) steps++;//0x45
            else return 0;
        case 7:
            if(SW6306_ByteRead(SW6306_CTRG_POSET, &SW6306_Status.pomax_set)) steps++;//0x4F
            else return 0;
        case 8:
            if(SW6306_ByteRead(SW6306_STRG_POMAX, &SW6306_Status.pomax)) steps++;//0x51
            else return 0;
        case 9:
            if(SW6306_ByteRead(SW6306_STRG_PIMAX, &SW6306_Status.pimax))//0x51
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}

uint16_t SW6306_ReadIPortLimit(void)//读取充电时端口限流实时值（单位：mA）
{
    return SW6306_Status.ibuslim_chg*50+200;
}
uint16_t SW6306_ReadIBattLimit(void)//读取充电时电池限流实时值（单位：mA）
{
    return SW6306_Status.ibuslim_chg*100+100;
}
uint8_t SW6306_ReadMaxOutputPower(void)//读取最大输出功率（单位：W）
{
    return SW6306_Status.pomax;
}
uint8_t SW6306_ReadMaxInputPower(void)//读取最大输入功率（单位：W）
{
    return SW6306_Status.pimax;
}

uint8_t SW6306_StatusLoad(void)//将SW6306的各种状态读取到镜像寄存器(0x12,0x14,0x15,0x18,0x1A,0x2A,0x2B,0x2C)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_STRG_MODE)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteRead(SW6306_STRG_MODE, &SW6306_Status.mode)) steps++;//0x12
            else return 0;
        case 2:
            if(SW6306_ByteRead(SW6306_STRG_DISPLAY, &SW6306_Status.display)) steps++;//0x14
            else return 0;
        case 3:
            if(SW6306_ByteRead(SW6306_STRG_FAULT0, &SW6306_Status.fault0)) steps++;//0x15
            else return 0;
        case 4:
            if(SW6306_ByteRead(SW6306_STRG_SYS_STAT, &SW6306_Status.sys_stat)) steps++;//0x18
            else return 0;
        case 5:
            if(SW6306_ByteRead(SW6306_STRG_NTC_CURR, &SW6306_Status.intc))//0x1A
            {
                switch(SW6306_Status.intc & SW6306_NTC_CURR_MSK)
                {
                    case SW6306_NTC_CURR_20U:
                        SW6306_Status.intc = 20;
                        return 1;
                    case SW6306_NTC_CURR_40U:
                        SW6306_Status.intc = 40;
                        return 1;
                    case SW6306_NTC_CURR_80U:
                        SW6306_Status.intc = 80;
                        return 1;
                    default:
                        return 0;
                }
                steps++;
            }
            else return 0;
        case 6:
            if(SW6306_ByteRead(SW6306_STRG_FAULT1, &SW6306_Status.fault1)) steps++;//0x2A
            else return 0;
        case 7:
            if(SW6306_ByteRead(SW6306_STRG_FAULT2, &SW6306_Status.fault2)) steps++;//0x2B
            else return 0;
        case 8:
            if(SW6306_ByteRead(SW6306_STRG_FAULT3, &SW6306_Status.fault3))//0x2C
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_IsLowCurrentMode(void)//SW6306是否处于小电流模式
{
    return SW6306_Status.mode & SW6306_MODE_BLUTH;
}
uint8_t SW6306_IsMPPTCharging(void)//SW6306是否处于MPPT充电模式
{
    return SW6306_Status.mode & SW6306_MODE_MPPTCHG;
}
uint8_t SW6306_IsCharging(void)//SW6306是否正在充电
{
    return SW6306_Status.sys_stat & SW6306_SYS_STAT_CHGING;
}
uint8_t SW6306_IsDischarging(void)//SW6306是否正在放电
{
    return SW6306_Status.sys_stat & SW6306_SYS_STAT_DISCHGING;
}
uint8_t SW6306_IsFullCharged(void)//SW6306是否充满
{
    return SW6306_Status.fault2 & SW6306_FAULT2_FULL;
}
uint8_t SW6306_IsBatteryDepleted(void)//SW6306电池是否耗尽
{
    return SW6306_Status.fault0 & SW6306_FAULT0_UVLO;
}
uint8_t SW6306_IsCapacityLearned(void)//是否已完成电量学习
{
    return SW6306_Status.fault0 & SW6306_FAULT0_LEARNEND;
}
uint8_t SW6306_IsErrorinCharging(void)//充电是否出现异常
{
    return SW6306_Status.fault0 & SW6306_FAULT0_CHGERR;
}
uint8_t SW6306_IsErrorinDischarging(void)//放电是否出现异常
{
    return SW6306_Status.fault0 & SW6306_FAULT0_DISCHGERR;
}
uint8_t SW6306_IsKeyEvent(void)//是否触发了按键事件
{
    return SW6306_Status.fault0 & SW6306_FAULT0_KEY;
}
uint8_t SW6306_IsSceneChanged(void)//是否发生场景变化
{
    return SW6306_Status.fault0 & SW6306_FAULT0_SCENE;
}
uint8_t SW6306_IsOverHeated(void)//是否发生过温异常
{
    return (SW6306_Status.fault1&(SW6306_FAULT1_OT_CHIP|SW6306_FAULT1_OT_NTC))|(SW6306_Status.fault2&(SW6306_FAULT2_OT_CHIP|SW6306_FAULT2_OT_NTC));
}
float SW6306_TNTC_Calc(void)//计算并返回NTC温度（未完成）
{
    return (float)SW6306_Status.vntc*1.1/SW6306_Status.intc;
}


uint8_t SW6306_CapacityLoad(void)//更新容量与库仑计镜像寄存器(0x86~0x8A,0x99,0xA2)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_STRG_BATLVL_DISPLAY)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteRead(SW6306_STRG_BATLVL_DISPLAY, &SW6306_Status.capacity)) steps++;//0x99
            else return 0;
        case 2:
            if(SW6306_ByteRead(SW6306_STRG_LEARN, &SW6306_Status.learn_stat)) steps++;//0xA2
            else return 0;
        case 3:
            if(SW6306_I2C_Receive(SW6306_I2C_ADDR, SW6306_CTRG_GAUGE_MCAPL, &SW6306_Status.maxcap, 2, &SW6306_Status.flag)) steps++;//0x86~0x87
            else return 0;
        case 4:
            if(SW6306_Status.flag) steps++;
            else return 0;
        case 5:
            if(SW6306_I2C_Receive(SW6306_I2C_ADDR, SW6306_CTRG_CURR_CAPL, &SW6306_Status.presentcap, 3, &SW6306_Status.flag)) steps++;//0x88~0x8A
            else return 0;
        case 6:
            if(SW6306_Status.flag)
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}
            
uint8_t SW6306_ReadCapacity(void)//读取SW6306显示电量
{
    return SW6306_Status.capacity;
}

float SW6306_ReadMaxGuageCap(void)//读取库仑计最大容量（单位：mAh）
{
    return SW6306_Status.maxcap*326.2236;
}

float SW6306_ReadPresentGuageCap(void)//读取库仑计当前容量（单位：mAh）
{
    return SW6306_Status.presentcap*0.07964;
}

uint8_t SW6306_Click(void)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_CTRG_CLICK)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteWrite(SW6306_CTRG_CLICK, SW6306_CLICK))
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_ForceOff()
{
    static uint8_t steps;
    switch(steps)
    {
        case 0://切换低地址
            if(SW6306_RegsetSwitch(SW6306_CTRG_DISCHG_OFF)) steps++;
            else return 0;
        case 1://关闭输出口
            if(SW6306_ByteWrite(SW6306_CTRG_DISCHG_OFF, SW6306_PORTEVT_C1_RMV|SW6306_PORTEVT_C2_RMV|SW6306_PORTEVT_A1_RMV|SW6306_PORTEVT_A2_RMV)) steps++;
            else return 0;
        case 2:
            if(SW6306_ByteWrite(SW6306_CTRG_DISCHG_OFF, SW6306_DISCHG_OFF))
            {
                steps++;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_Unlock()
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_CTRG_CLICK)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteWrite(SW6306_CTRG_CLICK, SW6306_CLICK)) steps++;
            else return 0;
        case 2:
            if(SW6306_ByteWrite(SW6306_CTRG_LPSET, SW6306_LPSET_EN)) steps++;
            else return 0;
        case 3:
            if(SW6306_ByteWrite(SW6306_CTRG_WREN, 0x20)) steps++;
            else return 0;
        case 4:
            if(SW6306_ByteWrite(SW6306_CTRG_WREN, 0x40)) steps++;
            else return 0;
        case 5:
            if(SW6306_ByteWrite(SW6306_CTRG_WREN, 0x80))
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_LPSet(void)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_CTRG_LPSET)) steps++;//切换低地址
            else return 0;
        case 1:
            if(SW6306_ByteModify(SW6306_CTRG_LPSET, SW6306_LPSET_EN, 0x00))
            {
                steps = 0;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_WLEDSet(uint8_t wledstatus)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_CTRG_MODE)) steps++;//切换低地址
            else return 0;
        case 1:
            if(wledstatus)
            {
                if(SW6306_ByteModify(SW6306_CTRG_MODE, SW6306_MODE_WLEDON, SW6306_MODE_WLEDON))
                {
                    steps = 0;
                    return 1;
                }
                else return 0;
            }
            else
            {
                if(SW6306_ByteModify(SW6306_CTRG_MODE, SW6306_MODE_WLEDON, 0x00))
                {
                    steps = 0;
                    return 1;
                }
                else return 0;
            }
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_IO1Set(uint8_t io1status)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0:
            if(SW6306_RegsetSwitch(SW6306_CTRG_IOCTL)) steps++;//切换低地址
            else return 0;
        case 1:
            if(io1status)
            {
                if(SW6306_ByteModify(SW6306_CTRG_IOCTL, SW6306_IO1ON, SW6306_IO1ON))
                {
                    steps = 0;
                    return 1;
                }
                else return 0;
            }
            else
            {
                if(SW6306_ByteModify(SW6306_CTRG_IOCTL, SW6306_IO1ON, 0x00))
                {
                    steps = 0;
                    return 1;
                }
                else return 0;
            }
        default:
            steps = 0;
            return 0;
    }
}

uint8_t SW6306_Init(void)
{
    static uint8_t steps;
    switch(steps)
    {
        case 0://切换寄存器组
            if(SW6306_RegsetSwitch(SW6306_CTRG_CLICK)) steps++;
            else return 0;
        case 1://触发一次短按键
            if(SW6306_ByteWrite(SW6306_CTRG_CLICK, SW6306_CLICK)) steps++;
            else return 0;
        case 2://解锁寄存器写入
            if(SW6306_Unlock()) steps++;
            else return 0;
        case 3://使能UVLO、充放电异常与场景变化中断
            if(SW6306_ByteWrite(SW6306_CTRG_INT_EN, SW6306_UVLO_INT_EN|SW6306_CHGERR_INT_EN|SW6306_DISCHGERR_INT_EN|SW6306_SCENE_INT_EN)) steps++;
            else return 0;
        case 4://IRQ脚拉低10ms
            if(SW6306_ByteWrite(SW6306_CTRG_IOCTL, SW6306_IRQ1)) steps++;
            else return 0;
        case 5://强制控制输入输出功率
            if(SW6306_ByteWrite(SW6306_CTRG_FORCECTL, SW6306_FORCECTL_POUT|SW6306_FORCECTL_PIN)) steps++;
            else return 0;
        case 6://输入功率设置
            if(SW6306_ByteWrite(SW6306_CTRG_PISET, SW6306_INPUT_POWER_MAX)) steps++;
            else return 0;
        case 7://输出功率设置
            if(SW6306_ByteWrite(SW6306_CTRG_POSET, SW6306_OUTPUT_POWER_MAX)) steps++;
            else return 0;
        case 8://输出功率设置
            if(SW6306_ByteModify(SW6306_STRG_LEARN, SW6306_LEARN_END, 0x00)) steps++;
            else return 0;
        case 9://切换寄存器组
            if(SW6306_RegsetSwitch(SW6306_CTRG_DCHG4)) steps++;
            else return 0;
        case 10://禁止放电恒温环
            if(SW6306_ByteWrite(SW6306_CTRG_DCHG0, SW6306_DCHG0_NOCT)) steps++;
            else return 0;
        case 11://放电UVLO 2.8V，0.2V迟滞
            if(SW6306_ByteModify(SW6306_CTRG_DCHG4, SW6306_DCHG4_MSK, SW6306_DCHG4_UVLOHYS_V2|SW6306_DCHG4_UVLO_2V8)) steps++;
            else return 0;
        case 12://4.2V电池，3S
            if(SW6306_ByteWrite(SW6306_CTRG_CHG1, SW6306_CHG1_BTYPE_REGSET|SW6306_CHG1_BTYPE_4V2|SW6306_CHG1_SERIES_REGSET|SW6306_CHG1_3S)) steps++;
            else return 0;
        case 13://涓流充电400mA，电压迟滞0.2V，48H超时
            if(SW6306_ByteWrite(SW6306_CTRG_CHG6, SW6306_CHG6_TCHG_400M|SW6306_CHG6_TCHGHYS_0V1|SW6306_CHG6_TCHG_48H)) steps++;
            else return 0;
        case 14://充电截止电流100mA，C口5V充电电流3.3A
            if(SW6306_ByteModify(SW6306_CTRG_CHG7, SW6306_CHG7_MSK, SW6306_CHG7_CHGEND_100M|SW6306_CHG7_C_3A3)) steps++;
            else return 0;
        case 15://B口5V充电电流2.3A，NTC-10°C低温保护，60°C高温保护
            if(SW6306_ByteWrite(SW6306_CTRG_CHG8, SW6306_CHG8_B_2A3|SW6306_CHG8_NTCUTP_M10C|SW6306_CHG8_NTCOTP_60C)) steps++;
            else return 0;
        case 16://触发62368高低温保护时减小功率为原来的1/2，高温范围10°C（60~50°C）
            if(SW6306_ByteWrite(SW6306_CTRG_CHG9, SW6306_CHG9_UT_4DIV8|SW6306_CHG9_OT_2DIV4|SW6306_CHG9_TR_10C)) steps++;
            else return 0;
        case 17://62368充电常温范围45°C
            if(SW6306_ByteWrite(SW6306_CTRG_CHG10, SW6306_CHG10_TN_45C)) steps++;
            else return 0;
        case 18://62368放电常温范围60~-10°C
            if(SW6306_ByteWrite(SW6306_CTRG_CHG11, SW6306_CHG11_TR_NTC)) steps++;
            else return 0;
        case 19://禁止充电恒温环，阈值100°C
            if(SW6306_ByteModify(SW6306_CTRG_CHG12, SW6306_CHG12_MSK, SW6306_CHG12_NOCT|SW6306_CHG12_CT_100C)) steps++;
            else return 0;
        case 20://BUS端下管Rdson 5mR
            if(SW6306_ByteModify(SW6306_CTRG_DCDC1, SW6306_DCDC1_MSK, SW6306_DCDC1_5R)) steps++;
            else return 0;
        case 21://充放电温度,62368功能,恒温环由寄存器控制
            if(SW6306_ByteModify(SW6306_CTRG_DCDC2, SW6306_DCDC2_MSK, SW6306_DCDC2_TEMP_REGSET|SW6306_DCDC2_62368_REGSET|SW6306_DCDC2_CT_REGSET)) steps++;
            else return 0;
        case 22://禁止A1,A2口VBUS接入检测
            if(SW6306_ByteModify(SW6306_CTRG_INDET0, SW6306_INDET0_MSK, SW6306_INDET0_NOA1VBUS|SW6306_INDET0_NOA2VBUS)) steps++;
            else return 0;
        case 23://禁止A1,A2口空载
            if(SW6306_ByteModify(SW6306_CTRG_INDET1, SW6306_INDET1_NOA1NL|SW6306_INDET1_NOA2NL, SW6306_INDET1_NOA1NL|SW6306_INDET1_NOA2NL)) steps++;
            else return 0;
        case 24://高压快充协议功率跟随系统设置
            if(SW6306_ByteModify(SW6306_CTRG_P_DPDM0, SW6306_P_DPDM0_MSK, SW6306_P_DPDM0_PSYS)) steps++;
            else return 0;
        case 25://QC3+协议最大功率45W,QC2协议最大电压20V
            if(SW6306_ByteWrite(SW6306_CTRG_P_DPDM1, SW6306_DPDM1_QC3P_45W|SW6306_DPDM1_QC2_20V)) steps++;
            else return 0;
        case 26://SFCP协议最大电压12V,PE协议最大电压12V
            if(SW6306_ByteModify(SW6306_CTRG_P_DPDM2, SW6306_DPDM2_MSK, SW6306_DPDM2_SFCP_12V|SW6306_DPDM2_PE_12V)) steps++;
            else return 0;
        case 27://VOOC协议使能
            if(SW6306_ByteModify(SW6306_CTRG_P_DPDM4, SW6306_P_DPDM4_MSK, SW6306_P_DPDM4_VOOC1|SW6306_P_DPDM4_VOOC4|SW6306_P_DPDM4_SVOOC)) steps++;
            else return 0;
        case 28://VOOC协议使能
            if(SW6306_ByteModify(SW6306_CTRG_P_DPDM5, SW6306_P_DPDM5_MSK, SW6306_P_DPDM5_VOOC|SW6306_P_DPDM5_SDP2A)) steps++;
            else return 0;
        case 29://数码管驱动电流5mA,轻载5s后关闭输出
            if(SW6306_ByteModify(SW6306_CTRG_DISPLAY, SW6306_DISPLAY_MSK, SW6306_DISPLAY_2_5M|SW6306_DISPLAY_5SDELAY)) steps++;
            else return 0;
        case 30://Rdc计算使能,容量学习使能
            if(SW6306_ByteModify(SW6306_CTRG_GAUGE0, SW6306_GAUGE0_MSK, SW6306_GAUGE0_RDCEN|SW6306_GAUGE0_LEARNEN)) steps++;
            else return 0;
        case 31://短按键打开灯显与已经接入的输出口,长按关闭下游口,双击打开WLED
            if(SW6306_ByteWrite(SW6306_CTRG_KEY1, SW6306_KEY1_DISPLAY|SW6306_KEY1_LPOFF|SW6306_KEY1_DCWLED)) steps++;
            else return 0;
        case 32://切换寄存器组
            if(SW6306_RegsetSwitch(SW6306_STRG_FAULT0)) steps++;
            else return 0;
        case 33://清标志位
            if(SW6306_ByteWrite(SW6306_STRG_FAULT0, SW6306_FAULT0_MSK))
            {
                steps = 0;
                SW6306_Status.initialized = 1;
                return 1;
            }
            else return 0;
        default:
            steps = 0;
            return 0;
    }
}
    
uint8_t SW6306_IsInitialized(void)//检测SW6306是否已初始化过，须在SW6306_PowerLoad()后执行
{
    if(SW6306_Status.initialized)
    {
        if((SW6306_Status.pimax_set==SW6306_INPUT_POWER_MAX)&&(SW6306_Status.pomax_set==SW6306_OUTPUT_POWER_MAX)) return 1;
        else
        {
            SW6306_Status.initialized = 0;
            return 0;
        }
    }
    else return 0;
}