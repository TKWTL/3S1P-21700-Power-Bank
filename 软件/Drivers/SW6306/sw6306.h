/*SW6306操作库 V0.0
/TKWTL 2024/07/06
*/
#ifndef __SW6306_H__
#define __SW6306_H__

#ifdef __cplusplus
extern C {
#endif

#include "stdint.h"
    
/******************************用户设置区开始**********************************/
/*包含自己的I2C驱动库*/
#include "main.h"

//允许库挂起以出让运行时间。该模式下，需要重复执行函数直到其返回1为止
#define SW6306_SUSPENDABLE

//外部库给出的I2C读写函数
#ifdef SW6306_SUSPENDABLE//允许挂起
    
#define SW6306_I2C_Transmit(addr,reg,pdata,len,pflag) ASYNC_I2C_Transmit(addr,reg,pdata,len,pflag)
#define SW6306_I2C_Receive(addr,reg,pdata,len,pflag) ASYNC_I2C_Receive(addr,reg,pdata,len,pflag)
    
    
#else//不允许挂起
    
    
    
#endif

//设置SW6306功率路径上的感测电阻值 (单位:mOhm)
#define SW_VBUS_RSHUNT                  5           //VBUS 电流路径上的感测电阻值
#define SW_BATT_RSHUNT                  5           //电池电流路径上的感测电阻值
    
//设置SW6306 NTC引脚上的电阻参数，用于计算温度
#define SW6306_NTC_B 3435

//输入输出最大功率设定，最大100W
#define SW6306_INPUT_POWER_MAX          60          //输入/充电功率
#define SW6306_OUTPUT_POWER_MAX         80          //输出/放电功率

/******************************用户设置区结束**********************************/

struct SW6306_StatusTypedef
{
    uint8_t online;                                                             //SW6306连接成功，表现为有设备响应0x3C的地址
    uint8_t initialized;                                                        //SW6306已初始化，当SW6306失去连接时自动回到未初始化状态
    uint8_t unlocked;                                                           //SW6306已解锁，此时可以对寄存器进行写入
    uint8_t sendbuf[2];                                                         //传输缓冲用变量
    uint8_t flag;                                                               //标识传输完成与传输状态用变量
    
/***************************寄存器内存镜像声明*********************************/
    //ADC读取数据区
    uint16_t vbus;                  //BUS电压
    uint16_t ibus;                  //BUS电流
    uint16_t vbat;                  //BAT电压
    uint16_t ibat;                  //BAT电流
    uint16_t tntc;                  //NTC温度（精度太低，不建议使用）
    uint16_t tchip;                 //芯片温度
    uint16_t vntc;                  //NTC电压
    //以上为原始数据，需要经过对应的Read()函数转换才有意义
    
    //状态寄存器区
    uint8_t vbus_chg;               //0x0E 充电电压状态
    uint8_t qcstat;                 //0x0F 充放电快充与种类指示
    uint8_t ibuslim_chg;            //0x10 充电时接口端实时限流值
    uint8_t ibatlim_chg;            //0x11 充电时电池端实时限流值
    uint8_t mode;                   //0x12 模式与状态指示
    uint8_t noload;                 //0x13 端口空载指示
    uint8_t display;                //0x14 LED与显示状态指示
    uint8_t fault0;                 //0x15 事件指示
    uint8_t sys_stat;               //0x18 系统状态指示
    uint8_t typec_stat;             //0x19 Type-C口状态
    uint8_t intc;                   //0x1A NTC电流挡位
    uint8_t typea_qcin;             //0x1C A口存在与快充电压挡位
    uint8_t port_stat;              //0x1D 端口状态指示
    uint8_t fault1;                 //0x2A 异常事件指示0
    uint8_t fault2;                 //0x2B 异常事件指示1
    uint8_t fault3;                 //0x2C 异常事件指示2
    uint8_t pomax;                  //0x51 最大输出功率
    uint8_t pimax;                  //0x52 最大输入功率
    uint16_t maxcap;                //0x86~0x87 库仑计最大容量
    uint32_t presentcap;            //0x88~0x8A 库伦计当前容量
    uint8_t capacity;               //0x99 显示电量
    uint8_t learn_stat;             //0xA2 容量学习状态指示
    
    //设置寄存器存档
    uint8_t pimax_set;              //0x45 最大输入功率
    uint8_t pomax_set;              //0x4F 最大输入功率
};

//SW6306 I2C 地址，为0x3C左移一位，最低位为0
#ifndef SW6306_I2C_ADDR
#define SW6306_I2C_ADDR                 0x78
#endif

/**************************SW6306 寄存器地址定义*******************************/
//命名规则：固定前缀(SW6306)_状态(ST)/控制(CT)寄存器(RG)_(功能描述)
#define SW6306_STRG_REV                 0x01U//版本号
#define SW6306_STRG_VBUS_CHG            0x0EU//充电电压状态
#define SW6306_STRG_QCSTAT              0x0FU//充放电快充与种类指示
#define SW6306_STRG_BUSILIM_CHG         0x10U//充电时接口端实时限流值
#define SW6306_STRG_BATILIM_CHG         0x11U//充电时电池端实时限流值
#define SW6306_STRG_MODE                0x12U//模式与状态指示
#define SW6306_STRG_NOLOAD              0x13U//端口空载指示
#define SW6306_STRG_DISPLAY             0x14U//LED与显示状态指示
#define SW6306_STRG_FAULT0              0x15U//事件指示
#define SW6306_STRG_SYS_STAT            0x18U//系统状态指示
#define SW6306_STRG_TYPEC               0x19U//Type-C口状态
#define SW6306_STRG_NTC_CURR            0x1AU//NTC电流指示
#define SW6306_STRG_TYPEA_QCIN          0x1CU//A口存在与快充电压挡位
#define SW6306_STRG_PORT_STA            0x1DU//端口状态指示

#define SW6306_CTRG_CLICK               0x20U//短按键事件触发（不会与外部按键形成双击，可用于保持唤醒）
#define SW6306_CTRG_DISCHG_OFF          0x21U//关闭放电
#define SW6306_CTRG_PORTEVT             0x22U//端口事件触发
#define SW6306_CTRG_LPSET               0x23U//低功耗设置（必须写0x01关闭低功耗以写入除0x21~0x24的寄存器）
#define SW6306_CTRG_WREN                0x24U//写寄存器使能与高位地址
#define SW6306_CTRG_INT_EN              0x25U//中断源使能
#define SW6306_CTRG_MODE                0x28U//模式设置
#define SW6306_CTRG_IOCTL               0x29U//IO0与IRQ脚模式控制

#define SW6306_STRG_FAULT1              0x2AU//异常事件指示0
#define SW6306_STRG_FAULT2              0x2BU//异常事件指示1
#define SW6306_STRG_FAULT3              0x2CU//异常事件指示2

#define SW6306_CTRG_CCDRV               0x2DU//CC驱动控制
#define SW6306_CTRG_PD_CMD              0x2EU//发送PD指令
#define SW6306_CTRG_UFCS_CMD            0x2FU//发送UFCS指令

#define SW6306_CTRG_ADC_SET             0x30U//ADC通道选择
#define SW6306_STRG_ADCL                0x31U//ADC数据低8位（设置完ADC通道后可立即读取）
#define SW6306_STRG_ADCH                0x32U//ADC数据高4位（可以从0x31连续读2byte）

#define SW6306_CTRG_FORCECTL            0x40U//强制控制使能
#define SW6306_CTRG_DCHG_VBUSL          0x41U//输出电压低8位
#define SW6306_CTRG_DCHG_VBUSH          0x42U//输出电压高4位
#define SW6306_CTRG_DCHG_IBUS           0x43U//输出端口限流
#define SW6306_CTRG_DCHG_IBAT           0x44U//输出电池限流
#define SW6306_CTRG_PISET               0x45U//输入功率
#define SW6306_CTRG_CHG_VBATL           0x46U//充电电压低8位
#define SW6306_CTRG_CHG_VBATH           0x47U//充电电压高4位
#define SW6306_CTRG_CHG_VHOLD           0x48U//充电欠压门限
#define SW6306_CTRG_CHG_IBUS            0x49U//充电端口限流
#define SW6306_CTRG_CHG_IBAT            0x4AU//充电电池限流
#define SW6306_CTRG_DISPLAY             0x4BU//灯显控制
#define SW6306_CTRG_WLSS_SET            0x4CU//无线充模式电压设置
#define SW6306_CTRG_POSET               0x4FU//输出功率

#define SW6306_CTRG_QCIN                0x50U//输入快充控制

#define SW6306_STRG_POMAX               0x51U//放电最大功率
#define SW6306_STRG_PIMAX               0x52U//充电最大功率

#define SW6306_CTRG_GAUGE_MCAPL         0x86U//库仑计最大容量低8位
#define SW6306_CTRG_GAUGE_MCAPH         0x87U//库仑计最大容量高4位
#define SW6306_CTRG_CURR_CAPL           0x88U//库仑计当前容量低8位
#define SW6306_CTRG_CURR_CAPM           0x89U//库仑计当前容量中8位
#define SW6306_CTRG_CURR_CAPH           0x8AU//库仑计当前容量高8位（可以从0x88连续读3byte）
#define SW6306_STRG_BATLVL_NOW          0x8BU//库仑计当前电量（百分比单位）
#define SW6306_STRG_BATLVL_AVAIL        0x8CU//库仑计可用电量（百分比单位）
#define SW6306_STRG_BATLVL_SMOOTH       0x94U//均匀化处理后的电量（百分比单位）（用途不明）
#define SW6306_STRG_BATLVL_DISPLAY      0x99U//显示电量（百分比单位）
#define SW6306_STRG_LEARN               0xA2U//容量学习状态指示

//外部系统区，使能后在此处实时更新电流值，使库仑计可以统计其他电路的电流，以便计算电池电量
#define SW6306_CTRG_EXTSYS_STA          0xA4U//外部系统状态标志
#define SW6306_CTRG_EXTSYS_IBATL        0xA5U//外部系统电池电流低8位
#define SW6306_CTRG_EXTSYS_IBATH        0xA6U//外部系统电池电流高4位
#define SW6306_CTRG_EXTSYS_ILIMBUS      0xA9U//外部系统端口输入限流
#define SW6306_CTRG_EXTSYS_ILIMBAT      0xAAU//外部系统电池限流
#define SW6306_CTRG_EXTSYS_IBUSL        0xABU//外部系统端口电流低8位
#define SW6306_CTRG_EXTSYS_IBUSH        0xACU//外部系统端口电流高4位

//0x100~0x1FF区，全为控制寄存器
#define SW6306_CTRG_DCHG0               0x100U//放电配置0
#define SW6306_CTRG_DCHG1               0x101U//放电配置1
#define SW6306_CTRG_DCHG2               0x102U//放电配置2
#define SW6306_CTRG_DCHG3               0x103U//放电配置3
#define SW6306_CTRG_DCHG4               0x104U//放电配置4
#define SW6306_CTRG_DCHG5               0x106U//放电配置5

#define SW6306_CTRG_CHG0                0x107U//充电配置0
#define SW6306_CTRG_CHG1                0x108U//充电配置1
#define SW6306_CTRG_CHG2                0x109U//充电配置2
#define SW6306_CTRG_CHG3                0x10AU//充电配置3
#define SW6306_CTRG_CHG4                0x10BU//充电配置4
#define SW6306_CTRG_CHG5                0x10CU//充电配置5
#define SW6306_CTRG_CHG6                0x10DU//充电配置6
#define SW6306_CTRG_CHG7                0x10EU//充电配置7
#define SW6306_CTRG_CHG8                0x10FU//充电配置8
#define SW6306_CTRG_CHG9                0x110U//充电配置9
#define SW6306_CTRG_CHG10               0x111U//充电配置10
#define SW6306_CTRG_CHG11               0x112U//充电配置11
#define SW6306_CTRG_CHG12               0x113U//充电配置12

#define SW6306_CTRG_DCDC0               0x114U//BUCK-BOOST变换器设置0
#define SW6306_CTRG_DCDC1               0x115U//BUCK-BOOST变换器设置1
#define SW6306_CTRG_DCDC2               0x116U//BUCK-BOOST变换器设置2

#define SW6306_CTRG_INDET0              0x117U//插拔检测设置0
#define SW6306_CTRG_INDET1              0x118U//插拔检测设置1
#define SW6306_CTRG_INDET2              0x119U//插拔检测设置2
#define SW6306_CTRG_INDET3              0x11AU//插拔检测设置3
#define SW6306_CTRG_INDET4              0x11BU//插拔检测设置4
#define SW6306_CTRG_INDET5              0x11DU//插拔检测设置5

#define SW6306_CTRG_MODEWLSS            0x11EU//无线充场景配置

#define SW6306_CTRG_PORTQC              0x11FU//端口快充配置
#define SW6306_CTRG_VCHG                0x120U//输入电压配置
#define SW6306_CTRG_P_DPDM0             0x122U//DPDM协议设置0
#define SW6306_CTRG_P_DPDM1             0x123U//DPDM协议设置1
#define SW6306_CTRG_P_DPDM2             0x124U//DPDM协议设置2
#define SW6306_CTRG_P_DPDM3             0x12AU//DPDM协议设置3
#define SW6306_CTRG_P_DPDM4             0x12BU//DPDM协议设置4
#define SW6306_CTRG_P_DPDM5             0x12CU//DPDM协议设置5

#define SW6306_CTRG_P_UFCS              0x12DU//UFCS协议设置
#define SW6306_CTRG_C_UFCS0             0x12EU//UFCS电流设置0
#define SW6306_CTRG_C_UFCS1             0x12FU//UFCS电流设置1
#define SW6306_CTRG_C_UFCS2             0x130U//UFCS电流设置2

#define SW6306_CTRG_TYPEC1              0x132U//C1 Type C设置
#define SW6306_CTRG_PD0                 0x133U//PD协议设置0
#define SW6306_CTRG_PD1                 0x134U//PD协议设置1
#define SW6306_CTRG_PD2                 0x135U//PD协议设置2
#define SW6306_CTRG_PD3                 0x136U//PD协议设置3
#define SW6306_CTRG_PD4                 0x137U//PD协议设置4
#define SW6306_CTRG_PD5                 0x138U//PD协议设置5
#define SW6306_CTRG_PD6                 0x139U//PD协议设置6
#define SW6306_CTRG_PD7                 0x13AU//PD协议设置7
#define SW6306_CTRG_PD8                 0x13BU//PD协议设置8
#define SW6306_CTRG_PD9                 0x13CU//PD协议设置9
#define SW6306_CTRG_PD10                0x13DU//PD协议设置10
#define SW6306_CTRG_PD11                0x13EU//PD协议设置11
#define SW6306_CTRG_PPS0                0x13FU//PPS电流设置0
#define SW6306_CTRG_PPS1                0x140U//PPS电流设置1
#define SW6306_CTRG_PPS2                0x141U//PPS电流设置2
#define SW6306_CTRG_PPS3                0x142U//PPS电流设置3
#define SW6306_CTRG_PD_VIDL             0x143U//PD VID低8位
#define SW6306_CTRG_PD_VIDH             0x144U//PD VID高8位
#define SW6306_CTRG_PD_BCDL             0x145U//PD BCD低8位
#define SW6306_CTRG_PD_BCDH             0x146U//PD BCD高8位
#define SW6306_CTRG_PD_PIDL             0x147U//PD PID低8位
#define SW6306_CTRG_PD_PIDH             0x148U//PD PID高8位
#define SW6306_CTRG_PD_SVIDL            0x149U//PD SVID低8位
#define SW6306_CTRG_PD_SVIDH            0x14AU//PD SVID高8位
#define SW6306_CTRG_TYPEC2              0x14BU//C2 Type C设置

#define SW6306_CTRG_DISPLAY             0x14DU//显示设置

#define SW6306_CTRG_GAUGE0              0x14EU//库仑计设置0
#define SW6306_CTRG_GAUGE1              0x14FU//库仑计设置1

#define SW6306_CTRG_KEY0                0x150U//按键设置0
#define SW6306_CTRG_KEY1                0x151U//按键设置1

#define SW6306_CTRG_IPPS                0x153U//PPS电流偏移设置

#define SW6306_CTRG_FAULT1              0x154U//异常/保护使能1
#define SW6306_CTRG_FAULT2              0x155U//异常/保护使能2
#define SW6306_CTRG_FAULT3              0x156U//异常/保护使能3

#define SW6306_CTRG_LREGSET             0x1FFU//写0切换低位寄存器地址



/******************************寄存器位定义************************************/

//0x0E  SW6306_STRG_VBUS_CHG        充电电压状态
#define SW6306_VBUS_CHG_REAL_MSK    0x70U//多口输入输出
#define SW6306_VBUS_CHG_REAL_20V    0x50U//VBUS实际电压20V
#define SW6306_VBUS_CHG_REAL_15V    0x40U//VBUS实际电压15V
#define SW6306_VBUS_CHG_REAL_12V    0x30U//VBUS实际电压12V
#define SW6306_VBUS_CHG_REAL_10V    0x20U//VBUS实际电压10V
#define SW6306_VBUS_CHG_REAL_9V     0x10U//VBUS实际电压9V
#define SW6306_VBUS_CHG_REAL_5V     0x00U//VBUS实际电压5V
#define SW6306_VBUS_CHG_AVAIL       0x08U//请求的实际电压有效
#define SW6306_VBUS_CHG_RQST_20V    0x05U//VBUS请求电压20V
#define SW6306_VBUS_CHG_RQST_15V    0x04U//VBUS请求电压15V
#define SW6306_VBUS_CHG_RQST_12V    0x03U//VBUS请求电压12V
#define SW6306_VBUS_CHG_RQST_10V    0x02U//VBUS请求电压10V
#define SW6306_VBUS_CHG_RQST_9V     0x01U//VBUS请求电压9V
#define SW6306_VBUS_CHG_RQST_5V     0x00U//VBUS请求电压5V

//0x0F  SW6306_STRG_QCSTAT          充放电快充与种类指示
#define SW6306_QCSTAT_VQC           0x20U//处于快充电压
#define SW6306_QCSTAT_PQC           0x10U//处于快充协议
#define SW6306_QCSTAT_UFCS          0x0FU//UFCS协议
#define SW6306_QCSTAT_AFC           0x0EU//AFC协议
#define SW6306_QCSTAT_SFCP          0x0DU//SFCP协议
#define SW6306_QCSTAT_SVOOC         0x0CU//Super VOOC协议
#define SW6306_QCSTAT_VOOC4         0x0BU//VOOC 4.0协议
#define SW6306_QCSTAT_VOOC1         0x0AU//VOOC 1.0协议
#define SW6306_QCSTAT_PE20          0x09U//MTK PE2.0协议
#define SW6306_QCSTAT_PE11          0x08U//MTK PE1.1协议
#define SW6306_QCSTAT_PDPPS         0x07U//PD协议，PPS电压
#define SW6306_QCSTAT_PDFIX         0x06U//PD协议，FIX电压
#define SW6306_QCSTAT_SCP           0x05U//SCP协议
#define SW6306_QCSTAT_FCP           0x04U//FCP协议
#define SW6306_QCSTAT_QC3P          0x03U//QC3+协议
#define SW6306_QCSTAT_QC3           0x02U//QC3协议
#define SW6306_QCSTAT_QC2           0x01U//QC2协议
#define SW6306_QCSTAT_NONE          0x00U//无协议

//0x12  SW6306_STRG_MODE            模式与状态指示
#define SW6306_MODE_WLSS_DISPL      0x08U//无线充工作指示灯显示
#define SW6306_MODE_WLSS_DISPA      0x04U//无线充异常指示灯显示
#define SW6306_MODE_BLUTH           0x02U//处于小电流模式
#define SW6306_MODE_MPPTCHG         0x01U//MPPT充电状态 

//0x13  SW6306_STRG_NOLOAD          端口空载指示
#define SW6306_NOLOAD_MSK           0x0FU//空载指示寄存器有效位
#define SW6306_A1_NOLOAD            0x08U//A1口处于空载
#define SW6306_A2_NOLOAD            0x04U//A2口处于空载
#define SW6306_C1_NOLOAD            0x02U//C1口处于空载
#define SW6306_C2_NOLOAD            0x01U//C2口处于空载

//0x14  SW6306_STRG_DISPLAY         LED与显示状态指示
#define SW6306_DISPLAY_MSK          0x30U//LED与显示状态指示有效位
#define SW6306_DISPLAY_WLED         0x20U//WLED打开
#define SW6306_DISPLAY_LED          0x10U//LED显示打开

//0x15  SW6306_STRG_FAULT0          事件指示
#define SW6306_FAULT0_MSK           0x3FU//事件指示寄存器有效位
#define SW6306_FAULT0_LEARNEND      0x20U//电量计量完成，写1清零
#define SW6306_FAULT0_UVLO          0x10U//电压欠压/UVLO事件指示，写1清零
#define SW6306_FAULT0_CHGERR        0x08U//充电异常事件指示，写1清零
#define SW6306_FAULT0_DISCHGERR     0x04U//放电异常事件指示，写1清零
#define SW6306_FAULT0_KEY           0x02U//按键事件指示，写1清零
#define SW6306_FAULT0_SCENE         0x01U//场景变化事件指示，写1清零

//0x18  SW6306_STRG_SYS_STAT        系统状态指示
#define SW6306_SYS_STAT_CHGERR      0x80U//异常导致充电关闭
#define SW6306_SYS_STAT_DISCHGERR   0x40U//异常导致放电关闭
#define SW6306_SYS_STAT_CHGING      0x20U//充电中
#define SW6306_SYS_STAT_DISCHGING   0x10U//放电中
#define SW6306_SYS_STAT_C1ON        0x08U//C1通路打开
#define SW6306_SYS_STAT_C2ON        0x04U//C2通路打开
#define SW6306_SYS_STAT_A1ON        0x02U//A1通路打开
#define SW6306_SYS_STAT_A2ON        0x01U//A2通路打开

//0x19  SW6306_STRG_TYPEC           Type C口状态
#define SW6306_TYPEC_C1MSK          0xC0U//C1口状态有效位
#define SW6306_TYPEC_C1SRC          0x80U//C1口作为source
#define SW6306_TYPEC_C1SNK          0x40U//C1口作为sink
#define SW6306_TYPEC_C2MSK          0x30U//C2口状态有效位
#define SW6306_TYPEC_C2SRC          0x20U//C2口作为source
#define SW6306_TYPEC_C2SNK          0x10U//C2口作为sink
#define SW6306_TYPEC_C1_CC1         0x08U//C1口CC1连接
#define SW6306_TYPEC_C1_CC2         0x04U//C1口CC2连接
#define SW6306_TYPEC_C2_CC1         0x02U//C2口CC1连接
#define SW6306_TYPEC_C2_CC2         0x01U//C2口CC2连接

//0x1A  SW6306_STRG_NTC_CURR        NTC电流指示
#define SW6306_NTC_CURR_MSK         0xC0U//NTC电流指示寄存器有效位
#define SW6306_NTC_CURR_20U         0x80U//NTC电流20uA
#define SW6306_NTC_CURR_40U         0x40U//NTC电流40uA
#define SW6306_NTC_CURR_80U         0x00U//NTC电流80uA

//0x1C  SW6306_STRG_TYPEA_QCIN      A口存在与快充电压挡位
#define SW6306_TYPEA_QCIN_MSK       0x7FU//A口存在与快充电压挡位寄存器有效位
#define SW6306_TYPEA_QCIN_A2EXIST   0x40U//A2口存在
#define SW6306_TYPEA_QCIN_A1EXIST   0x20U//A1口存在
#define SW6306_TYPEA_QCIN_9VEN      0x10U//当前协议支持9V挡位快充
#define SW6306_TYPEA_QCIN_10VEN     0x08U//当前协议支持10V挡位快充
#define SW6306_TYPEA_QCIN_12VEN     0x04U//当前协议支持12V挡位快充
#define SW6306_TYPEA_QCIN_15VEN     0x02U//当前协议支持15V挡位快充
#define SW6306_TYPEA_QCIN_20VEN     0x01U//当前协议支持20V挡位快充

//0x1D  SW6306_STRG_PORT_STA        端口状态指示
#define SW6306_PORT_STA_C1SRC       0x80U//C1端口作为source连接
#define SW6306_PORT_STA_C1SNK       0x40U//C1端口作为sink连接
#define SW6306_PORT_STA_C2SRC       0x20U//C2端口作为source连接
#define SW6306_PORT_STA_C2SNK       0x10U//C2端口作为sink连接
#define SW6306_PORT_STA_A1OL        0x08U//A1端口在线
#define SW6306_PORT_STA_A2OL        0x04U//A2端口在线
#define SW6306_PORT_STA_C1OL        0x02U//C1端口在线
#define SW6306_PORT_STA_C2OL        0x01U//C2端口在线

//0x20  SW6306_CTRG_CLICK           短按键事件触发
#define SW6306_CLICK                0x01U//触发一次短按键事件

//0x21  SW6306_CTRG_DISCHG_OFF      关闭放电
#define SW6306_DISCHG_OFF           0x20U//写1关闭放电，自动清零

//0x22  SW6306_CTRG_PORTEVT         端口事件触发（对于C口仅在其为source状态时生效）
#define SW6306_PORTEVT_C1_RMV       0x80U//触发C1口的负载拔出
#define SW6306_PORTEVT_C1_INS       0x40U//触发C1口的负载插入
#define SW6306_PORTEVT_C2_RMV       0x20U//触发C2口的负载拔出
#define SW6306_PORTEVT_C2_INS       0x10U//触发C2口的负载插入
#define SW6306_PORTEVT_A1_RMV       0x08U//触发A1口的负载拔出
#define SW6306_PORTEVT_A1_INS       0x04U//触发A1口的负载插入
#define SW6306_PORTEVT_A2_RMV       0x02U//触发A2口的负载拔出
#define SW6306_PORTEVT_A2_INS       0x01U//触发A2口的负载插入

//0x23  SW6306_CTRG_LPSET           低功耗设置
#define SW6306_LPSET_EN             0x01U//低功耗关闭

//0x25  SW6306_CTRG_INT_EN          中断源使能
#define SW6306_LEARNED_INT_EN       0x20U//电量计量完成中断使能
#define SW6306_UVLO_INT_EN          0x10U//UVLO中断使能
#define SW6306_CHGERR_INT_EN        0x08U//充电异常中断使能
#define SW6306_DISCHGERR_INT_EN     0x04U//放电异常中断使能
#define SW6306_KEY_INT_EN           0x02U//按键事件中断使能
#define SW6306_SCENE_INT_EN         0x01U//场景变化中断使能

//0x28  SW6306_CTRG_MODE            模式设置
#define SW6306_MODE_WLEDON          0x10U//打开WLED
#define SW6306_MODE_DISABLEOUTPUT   0x08U//强制关闭输出
#define SW6306_MODE_DISABLECHARGE   0x04U//强制关闭充电
#define SW6306_MODE_BLUTHON         0x02U//进入小电流模式（先写0再写1）
#define SW6306_MODE_MPPTON          0x01U//打开MPPT充电

//0x29  SW6306_CTRG_IOCTL           IO0与IRQ脚模式控制
#define SW6306_IO1ON                0x04U//IO0脚控制
#define SW6306_IRQ0                 0x00U//IRQ脚模式控制：LED/数码管工作时持续拉低
#define SW6306_IRQ1                 0x01U//IRQ脚模式控制：中断事件使能且发生后，发送10mS低电平脉冲
#define SW6306_IRQ2                 0x02U//IRQ脚模式控制：中断事件使能且发生后，持续拉低直到对应中断标志位清除 
#define SW6306_IRQ3                 0x03U//IRQ脚模式控制：无动作

//0x2A  SW6306_STRG_FAULT1          异常事件指示0（放电指示）
#define SW6306_FAULT1_MSK           0x7FU//异常事件指示0有效位
#define SW6306_FAULT1_OV_BAT        0x40U//放电时电池过压异常
#define SW6306_FAULT1_OT_CHIP       0x20U//放电时芯片过温
#define SW6306_FAULT1_OT_NTC        0x10U//放电时NTC过温
#define SW6306_FAULT1_OL_BUS        0x08U//放电时VBUS过载
#define SW6306_FAULT1_SHT_BUS       0x04U//放电时VBUS短路
#define SW6306_FAULT1_SOV_BUS       0x02U//放电时VBUS慢速过压
#define SW6306_FAULT1_FOV_BUS       0x01U//放电时VBUS快速过压

//0x2B  SW6306_STRG_FAULT2          异常事件指示1（充电指示）
#define SW6306_FAULT2_BATLOW        0x80U//电池电压低于1.5V
#define SW6306_FAULT2_OT            0x40U//充电超时
#define SW6306_FAULT2_FULL          0x20U//已充满
#define SW6306_FAULT2_OV_BAT        0x10U//充电过压
#define SW6306_FAULT2_OT_CHIP       0x08U//充电时芯片过温
#define SW6306_FAULT2_OT_NTC        0x04U//充电时NTC过温
#define SW6306_FAULT2_SOV_BUS       0x02U//充电时VBUS慢速过压
#define SW6306_FAULT2_FOV_BUS       0x01U//充电时VBUS快速过压

//0x2C  SW6306_STRG_FAULT3          异常事件指示2
#define SW6306_FAULT3_MSK           0xDFU//异常事件指示2有效位
#define SW6306_FAULT3_LONGPRESS     0xC0U//发生长按
#define SW6306_FAULT3_DOUBLECLICK   0x80U//发生双击
#define SW6306_FAULT3_SHORTPRESS    0x40U//发生短按
#define SW6306_FAULT3_NULLKEY       0x00U//无按键事件
#define SW6306_FAULT3_DCHG_62368UT  0x10U//放电时发生62368低温异常
#define SW6306_FAULT3_CHG_62368UT   0x08U//充电时发生62368低温异常
#define SW6306_FAULT3_CHG_62368OT   0x04U//充电时发生62368高温异常
#define SW6306_FAULT3_OV_DPDM       0x02U//放电时发生DPDM 5.5V过压
#define SW6306_FAULT3_OV_CC         0x01U//放电时发生CC 5.5V过压

//0x2D  SW6306_CTRG_CCDRV           CC驱动控制
#define SW6306_CCDRV_MSK            0x03U//CC驱动控制有效位
#define SW6306_CCDRV_C1OFF          0x02U//暂时关闭C1口的CC驱动
#define SW6306_CCDRV_C2OFF          0x01U//暂时关闭C2口的CC驱动

//0x2E  SW6306_CTRG_PD_CMD          发送PD指令
#define SW6306_PD_CMD_MSK           0x0FU//发送PD指令寄存器有效位
#define SW6306_PD_CMD_HARDRST       0x02U//启动发送hard reset的流程
#define SW6306_PD_CMD_SRCCAP        0x01U//重新发送source cap
#define SW6306_PD_CMD_NONE          0x00U//无影响

//0x2F  SW6306_CTRG_UFCS_CMD        发送UFCS指令
#define SW6306_UFCS_CMD_SRCCAP      0x01U//重新发送source cap

//0x30  SW6306_CTRG_ADC_SET         ADC通道选择
#define SW6306_ADC_SET_VBUS         0x00U//ADC通道选择输入输出电压（8mV）
#define SW6306_ADC_SET_IBUS         0x01U//ADC通道选择输入/输出IBUS电流（4mA）
#define SW6306_ADC_SET_VBAT         0x02U//ADC通道选择电池电压（7mV）
#define SW6306_ADC_SET_IBAT         0x03U//ADC通道选择输入/输出IBAT电流（5mA）
#define SW6306_ADC_SET_TNTC         0x04U//ADC通道选择NTC温度 (5℃) 
#define SW6306_ADC_SET_TCHIP        0x09U//ADC通道选择芯片温度(1/6.82℃) 
#define SW6306_ADC_SET_VNTC         0x0AU//ADC通道选择NTC电压（1.1mV）

//0x40  SW6306_CTRG_FORCECTL        强制控制使能
#define SW6306_FORCECTL_POUT        0x80U//使能I2C强制控制输出功率(0x4F寄存器)
#define SW6306_FORCECTL_IBUS        0x40U//使能I2C强制控制BUS限流(0x43 0x49寄存器)
#define SW6306_FORCECTL_IBAT        0x20U//使能I2C强制控制BAT限流(0x44 0x4A寄存器)
#define SW6306_FORCECTL_HOLD        0x08U//使能I2C强制控制充电HOLD门限(0x48寄存器)
#define SW6306_FORCECTL_PIN         0x04U//使能I2C强制控制输入功率(0x45寄存器)
#define SW6306_FORCECTL_VBUS        0x02U//使能I2C强制控制BUS电压(0x46 0x47寄存器)
#define SW6306_FORCECTL_VBAT        0x01U//使能I2C强制控制BAT目标电压(0x41 0x42寄存器)

//0x4B  SW6306_CTRG_DISPLAY         灯显控制
#define SW6306_DISPLAY_MSK　        0x0FU//灯显控制寄存器有效位
#define SW6306_DISPLAY_FAULT        0x08U//异常灯显
#define SW6306_DISPLAY_QC           0x04U//快充灯显
#define SW6306_DISPLAY_BLUTH        0x02U//小电流灯显
#define SW6306_DISPLAY_WLSS         0x01U//无线充灯显

//0x4C  SW6306_CTRG_WLSS_SET        无线充模式电压设置
#define SW6306_WLSS_SET_MSK         0x0CU//无线充模式电压设置寄存器有效位
#define SW6306_WLSS_VINREGSET       0x08U//无线充模式下申请输入的电压由寄存器0x50设置
#define SW6306_WLSS_VOUTREGSET      0x04U//无线充模式下输出的电压由寄存器0x41 0x42设置

//0x50  SW6306_CTRG_QCIN            输入快充控制
#define SW6306_QCIN_VINREGSET       0x08U//输入快充申请电压由该寄存器控制
#define SW6306_QCIN_20VRSQ          0x05U//输入快充申请20V电压
#define SW6306_QCIN_15VRSQ          0x04U//输入快充申请15V电压
#define SW6306_QCIN_12VRSQ          0x03U//输入快充申请12V电压
#define SW6306_QCIN_10VRSQ          0x02U//输入快充申请10V电压
#define SW6306_QCIN_9VRSQ           0x01U//输入快充申请9V电压
#define SW6306_QCIN_5VRSQ           0x00U//输入快充申请5V电压

//0xA2  SW6306_STRG_LEARN           容量学习状态指示
#define SW6306_LEARN_MSK            0x60U//容量学习状态指示寄存器有效位
#define SW6306_LEARN_ING            0x40U//容量学习进行中
#define SW6306_LEARN_END            0x20U//容量学习已完成

//0xA4  SW6306_CTRG_EXTSYS_STA      外部系统状态标志
#define SW6306_EXTSYS_STA_MSK       0x0FU//外部系统状态标志寄存器有效位
#define SW6306_EXTSYS_STA_EN        0x08U//外部系统写入数据有效
#define SW6306_EXTSYS_STA_DIS       0x04U//外部系统放电中
#define SW6306_EXTSYS_STA_FULL      0x02U//外部系统已充满
#define SW6306_EXTSYS_STA_LOW       0x01U//外部系统低电量

//0x100 SW6306_CTRG_DCHG0           放电配置0
#define SW6306_DCHG0_NOCT           0x80U//禁止放电恒温环
#define SW6306_DCHG0_130CT          0x70U//放电恒温环130°C
#define SW6306_DCHG0_120CT          0x60U//放电恒温环120°C
#define SW6306_DCHG0_110CT          0x50U//放电恒温环110°C
#define SW6306_DCHG0_100CT          0x40U//放电恒温环100°C
#define SW6306_DCHG0_90CT           0x30U//放电恒温环90°C
#define SW6306_DCHG0_80CT           0x20U//放电恒温环80°C
#define SW6306_DCHG0_70CT           0x10U//放电恒温环70°C
#define SW6306_DCHG0_60CT           0x00U//放电恒温环60°C
#define SW6306_DCHG0_POREGSET       0x08U//输出最大功率由该寄存器控制（0x40寄存器对应的强制控制位使能时不生效）
#define SW6306_DCHG0_PO100W         0x06U//输出最大功率100W
#define SW6306_DCHG0_PO65W          0x05U//输出最大功率65W
#define SW6306_DCHG0_PO60W          0x04U//输出最大功率60W
#define SW6306_DCHG0_PO45W          0x03U//输出最大功率45W
#define SW6306_DCHG0_PO35W          0x02U//输出最大功率35W
#define SW6306_DCHG0_PO30W          0x01U//输出最大功率30W
#define SW6306_DCHG0_PO27W          0x00U//输出最大功率27W

//0x101 SW6306_CTRG_DCHG1           放电配置1
#define SW6306_DCHG1_MOUT_6A2       0xC0U//多口输出总限流6.2A
#define SW6306_DCHG1_MOUT_5A2       0x80U//多口输出总限流5.2A
#define SW6306_DCHG1_MOUT_3A        0x40U//多口输出总限流3.0A
#define SW6306_DCHG1_MOUT_4A2       0x00U//多口输出总限流4.2A
#define SW6306_DCHG1_IBUS_600M      0x30U//BUS限流偏移600mA
#define SW6306_DCHG1_IBUS_450M      0x20U//BUS限流偏移450mA
#define SW6306_DCHG1_IBUS_150M      0x10U//BUS限流偏移150mA
#define SW6306_DCHG1_IBUS_300M      0x00U//BUS限流偏移300mA
#define SW6306_DCHG1_VBUS_300M      0x0CU//BUS电压偏移300mV
#define SW6306_DCHG1_VBUS_200M      0x08U//BUS电压偏移200mV
#define SW6306_DCHG1_VBUS_0M        0x04U//BUS电压偏移0mV
#define SW6306_DCHG1_VBUS_100M      0x00U//BUS电压偏移100mV
#define SW6306_DCHG1_COMP_80M       0x03U//线损补偿80mV/A
#define SW6306_DCHG1_COMP_100M      0x02U//线损补偿100mV/A
#define SW6306_DCHG1_COMP_0M        0x01U//无线损补偿
#define SW6306_DCHG1_COMP_60M       0x00U//线损补偿60mV/A

//0x102 SW6306_CTRG_DCHG2           放电配置2
#define SW6306_DCHG2_NOCOMP         0x80U//禁止快充线损补偿
#define SW6306_DCHG2_NOFOVP         0x04U//禁止VBUS快速过压保护

//0x103 SW6306_CTRG_DCHG3           放电配置3
#define SW6306_DCHG3_IBATLIM_NO     0xC0U//放电BAT限流不生效
#define SW6306_DCHG3_IBATLIM_8A     0x80U//放电BAT限流8A
#define SW6306_DCHG3_IBATLIM_10A    0x40U//放电BAT限流10A
#define SW6306_DCHG3_IBATLIM_12A    0x00U//放电BAT限流12A
#define SW6306_DCHG3_LFP_3V0        0x0CU//磷酸铁锂单节3.0V欠压
#define SW6306_DCHG3_LFP_2V9        0x0BU//磷酸铁锂单节2.9V欠压
#define SW6306_DCHG3_LFP_2V8        0x0AU//磷酸铁锂单节2.8V欠压
#define SW6306_DCHG3_LFP_2V7        0x09U//磷酸铁锂单节2.7V欠压
#define SW6306_DCHG3_LFP_2V6        0x08U//磷酸铁锂单节2.6V欠压
#define SW6306_DCHG3_LFP_2V5        0x07U//磷酸铁锂单节2.5V欠压
#define SW6306_DCHG3_LFP_2V4        0x06U//磷酸铁锂单节2.4V欠压
#define SW6306_DCHG3_LFP_2V3        0x05U//磷酸铁锂单节2.3V欠压
#define SW6306_DCHG3_LFP_2V2        0x04U//磷酸铁锂单节2.2V欠压
#define SW6306_DCHG3_LFP_2V1        0x03U//磷酸铁锂单节2.1V欠压
#define SW6306_DCHG3_LFP_2V0        0x02U//磷酸铁锂单节2.0V欠压
#define SW6306_DCHG3_LFP_1V9        0x01U//磷酸铁锂单节1.9V欠压
#define SW6306_DCHG3_LFP_2V75       0x00U//磷酸铁锂单节2.75V欠压

//0x104 SW6306_CTRG_DCHG4           放电配置4
#define SW6306_DCHG4_MSK            0x77U//放电配置4寄存器有效位
#define SW6306_DCHG4_UVLOHYS_V8     0x70U//放电UVLO单节迟滞电压0.8V
#define SW6306_DCHG4_UVLOHYS_V0     0x60U//放电UVLO单节迟滞电压0V
#define SW6306_DCHG4_UVLOHYS_V6     0x50U//放电UVLO单节迟滞电压0.6V
#define SW6306_DCHG4_UVLOHYS_V5     0x40U//放电UVLO单节迟滞电压0.5V
#define SW6306_DCHG4_UVLOHYS_V3     0x30U//放电UVLO单节迟滞电压0.3V
#define SW6306_DCHG4_UVLOHYS_V2     0x20U//放电UVLO单节迟滞电压0.2V
#define SW6306_DCHG4_UVLOHYS_V1     0x10U//放电UVLO单节迟滞电压0.1V
#define SW6306_DCHG4_UVLOHYS_V4     0x00U//放电UVLO单节迟滞电压0.4V
#define SW6306_DCHG4_UVLO_3V3       0x07U//放电UVLO单节电压3.3V
#define SW6306_DCHG4_UVLO_3V2       0x06U//放电UVLO单节电压3.2V
#define SW6306_DCHG4_UVLO_3V1       0x05U//放电UVLO单节电压3.1V
#define SW6306_DCHG4_UVLO_3V9       0x04U//放电UVLO单节电压3.9V
#define SW6306_DCHG4_UVLO_2V8       0x03U//放电UVLO单节电压2.8V
#define SW6306_DCHG4_UVLO_2V7       0x02U//放电UVLO单节电压2.7V
#define SW6306_DCHG4_UVLO_2V6       0x01U//放电UVLO单节电压2.6V
#define SW6306_DCHG4_UVLO_3V0       0x00U//放电UVLO单节电压3.0V

//0x106 SW6306_CTRG_DCHG5           放电配置5
#define SW6306_DCHG5_L10U           0x80U//电感10uH
#define SW6306_DCHG5_L4U7           0x00U//电感4.7uH
#define SW6306_DCHG5_NTCOTP_65C     0x30U//放电NTC高温限制65°C
#define SW6306_DCHG5_NTCOTP_55C     0x20U//放电NTC高温限制55°C
#define SW6306_DCHG5_NTCOTP_50C     0x10U//放电NTC高温限制50°C
#define SW6306_DCHG5_NTCOTP_60C     0x00U//放电NTC高温限制60°C
#define SW6306_DCHG5_NTCUTP_M30C    0x07U//放电NTC低温限制-30°C
#define SW6306_DCHG5_NTCUTP_M25C    0x06U//放电NTC低温限制-25°C
#define SW6306_DCHG5_NTCUTP_NO      0x05U//放电NTC低温不生效
#define SW6306_DCHG5_NTCUTP_0C      0x04U//放电NTC低温限制0°C
#define SW6306_DCHG5_NTCUTP_M5C     0x03U//放电NTC低温限制-5°C
#define SW6306_DCHG5_NTCUTP_M10C    0x02U//放电NTC低温限制-10°C
#define SW6306_DCHG5_NTCUTP_M15C    0x01U//放电NTC低温限制-15°C
#define SW6306_DCHG5_NTCUTP_M20C    0x00U//放电NTC低温限制-20°C

//0x107 SW6306_CTRG_CHG0            充电配置0
#define SW6306_CHG0_PFIX            0x10U//快充按照固定功率充电
#define SW6306_CHG0_ILIM            0x00U//快充按照允许的最大电流充电
#define SW6306_CHG0_PIREGSET        0x08U//输入最大功率由该寄存器控制（0x40寄存器对应的强制控制位使能时不生效）
#define SW6306_CHG0_PI100W          0x06U//输入最大功率100W
#define SW6306_CHG0_PI65W           0x05U//输入最大功率65W
#define SW6306_CHG0_PI60W           0x04U//输入最大功率60W
#define SW6306_CHG0_PI45W           0x03U//输入最大功率45W
#define SW6306_CHG0_PI35W           0x02U//输入最大功率35W
#define SW6306_CHG0_PI30W           0x01U//输入最大功率30W
#define SW6306_CHG0_PI27W           0x00U//输入最大功率27W

//0x108 SW6306_CTRG_CHG1            充电配置1
#define SW6306_CHG1_BTYPE_REGSET    0x80U//电池最大电压由该寄存器设置
#define SW6306_CHG1_BTYPE_3V65      0x60U//电池充电限制电压3.65V
#define SW6306_CHG1_BTYPE_3V6       0x50U//电池充电限制电压3.6V
#define SW6306_CHG1_BTYPE_4V5       0x40U//电池充电限制电压4.5V
#define SW6306_CHG1_BTYPE_4V4       0x30U//电池充电限制电压4.4V
#define SW6306_CHG1_BTYPE_4V35      0x20U//电池充电限制电压4.35V
#define SW6306_CHG1_BTYPE_4V3       0x10U//电池充电限制电压4.3V
#define SW6306_CHG1_BTYPE_4V2       0x00U//电池充电限制电压4.2V
#define SW6306_CHG1_SERIES_REGSET   0x08U//电池串联数目由该寄存器设置
#define SW6306_CHG1_7S              0x07U//7串电池
#define SW6306_CHG1_6S              0x06U//6串电池
#define SW6306_CHG1_5S              0x05U//5串电池
#define SW6306_CHG1_4S              0x04U//4串电池
#define SW6306_CHG1_3S              0x03U//3串电池
#define SW6306_CHG1_2S              0x02U//2串电池

//0x109 SW6306_CTRG_CHG2            充电配置2
#define SW6306_CHG2_NOU1V5          0x80U//电池电压1.5V以下不允许充电
#define SW6306_CHG2_ENU1V5          0x00U//电池电压1.5V以下允许充电
#define SW6306_CHG2_4V9             0x70U//5V低压限压4.9V
#define SW6306_CHG2_4V8             0x60U//5V低压限压4.8V
#define SW6306_CHG2_4V7             0x50U//5V低压限压4.7V
#define SW6306_CHG2_4V6             0x40U//5V低压限压4.6V（默认）
#define SW6306_CHG2_4V5             0x30U//5V低压限压4.5V
#define SW6306_CHG2_4V4             0x20U//5V低压限压4.4V
#define SW6306_CHG2_4V3             0x10U//5V低压限压4.3V
#define SW6306_CHG2_4V2             0x00U//5V低压限压4.2V
#define SW6306_CHG2_NOTCOT          0x08U//关闭涓流充电超时
#define SW6306_CHG2_ENTCOT          0x00U//涓流充电40分钟超时
#define SW6306_CHG2_8V9             0x07U//9V低压限压8.9V
#define SW6306_CHG2_8V8             0x06U//9V低压限压8.8V
#define SW6306_CHG2_8V7             0x05U//9V低压限压8.7V
#define SW6306_CHG2_8V6             0x04U//9V低压限压8.6V
#define SW6306_CHG2_8V5             0x03U//9V低压限压8.5V（默认）
#define SW6306_CHG2_8V4             0x02U//9V低压限压8.4V
#define SW6306_CHG2_8V3             0x01U//9V低压限压8.3V
#define SW6306_CHG2_8V2             0x00U//9V低压限压8.2V

//0x10A SW6306_CTRG_CHG3            充电配置3
#define SW6306_CHG3_NOADP           0x80U//无适配器拉挂保护
#define SW6306_CHG3_ENADP           0x00U//适配器拉挂保护使能
#define SW6306_CHG3_9V9             0x70U//9V高压限压9.9V
#define SW6306_CHG3_9V8             0x60U//9V高压限压9.8V
#define SW6306_CHG3_9V7             0x50U//9V高压限压9.7V
#define SW6306_CHG3_9V6             0x40U//9V高压限压9.6V
#define SW6306_CHG3_9V5             0x30U//9V高压限压9.5V（默认）
#define SW6306_CHG3_9V4             0x20U//9V高压限压9.4V
#define SW6306_CHG3_9V3             0x10U//9V高压限压9.3V
#define SW6306_CHG3_9V2             0x00U//9V高压限压9.2V
#define SW6306_CHG3_NOCHGDIDCHG     0x08U//无5V边充边放
#define SW6306_CHG3_ENCHGDIDCHG     0x00U//5V边充边放使能
#define SW6306_CHG3_11V9            0x07U//12V低压限压11.9V
#define SW6306_CHG3_11V8            0x06U//12V低压限压11.8V
#define SW6306_CHG3_11V7            0x05U//12V低压限压11.7V
#define SW6306_CHG3_11V6            0x04U//12V低压限压11.6V
#define SW6306_CHG3_11V5            0x03U//12V低压限压11.5V（默认）
#define SW6306_CHG3_11V4            0x02U//12V低压限压11.4V
#define SW6306_CHG3_11V3            0x01U//12V低压限压11.3V
#define SW6306_CHG3_11V2            0x00U//12V低压限压11.2V

//0x10B SW6306_CTRG_CHG4            充电配置4
#define SW6306_CHG4_NOEXTNOLD       0x80U//边充边放时输入欠压不变
#define SW6306_CHG4_ENEXTNOLD       0x00U//边充边放时输入欠压提高到4.8V
#define SW6306_CHG4_14V5            0x70U//15V低压限压14.5V（默认）
#define SW6306_CHG4_14V4            0x60U//15V低压限压14.4V
#define SW6306_CHG4_14V3            0x50U//15V低压限压14.3V
#define SW6306_CHG4_14V2            0x40U//15V低压限压14.2V
#define SW6306_CHG4_14V1            0x30U//15V低压限压14.1V
#define SW6306_CHG4_14V0            0x20U//15V低压限压14.0V
#define SW6306_CHG4_13V9            0x10U//15V低压限压13.9V
#define SW6306_CHG4_13V8            0x00U//15V低压限压13.8V
#define SW6306_CHG4_19V5            0x07U//20V低压限压19.5V
#define SW6306_CHG4_19V4            0x06U//20V低压限压19.4V
#define SW6306_CHG4_19V3            0x05U//20V低压限压19.3V
#define SW6306_CHG4_19V2            0x04U//20V低压限压19.2V
#define SW6306_CHG4_19V1            0x03U//20V低压限压19.1V
#define SW6306_CHG4_19V0            0x02U//20V低压限压19.0V（默认）
#define SW6306_CHG4_18V9            0x01U//20V低压限压18.9V
#define SW6306_CHG4_18V8            0x00U//20V低压限压18.8V

//0x10C SW6306_CTRG_CHG5            充电配置5
#define SW6306_CHG5_NOLOWHOLD       0x08U//充电电流大于3A时不降低欠压门限
#define SW6306_CHG5_ENLOWHOLD       0x00U//充电电流大于3A时降低欠压门限到当前挡位最小值

//0x10D SW6306_CTRG_CHG6            充电配置6（涓流充电）
#define SW6306_CHG6_TCHG_2V8        0xC0U//磷酸铁锂涓流充电门限2.8V
#define SW6306_CHG6_TCHG_2V5        0x80U//磷酸铁锂涓流充电门限2.5V
#define SW6306_CHG6_TCHG_2V1        0x40U//磷酸铁锂涓流充电门限2.1V
#define SW6306_CHG6_TCHG_2V75       0x00U//磷酸铁锂涓流充电门限2.75V
#define SW6306_CHG6_TCHG_400M       0x30U//涓流充电电流400mA
#define SW6306_CHG6_TCHG_300M       0x20U//涓流充电电流300mA
#define SW6306_CHG6_TCHG_200M       0x10U//涓流充电电流200mA
#define SW6306_CHG6_TCHG_100M       0x00U//涓流充电电流100mA
#define SW6306_CHG6_TCHGHYS_0V4     0x0CU//涓流充电电压迟滞0.4V
#define SW6306_CHG6_TCHGHYS_0V3     0x08U//涓流充电电压迟滞0.3V
#define SW6306_CHG6_TCHGHYS_0V2     0x04U//涓流充电电压迟滞0.2V
#define SW6306_CHG6_TCHGHYS_0V1     0x00U//涓流充电电压迟滞0.1V
#define SW6306_CHG6_TCHG_INF        0x03U//涓流充电无限时间
#define SW6306_CHG6_TCHG_48H        0x02U//涓流充电48H超时
#define SW6306_CHG6_TCHG_33H        0x01U//涓流充电33H超时
#define SW6306_CHG6_TCHG_22H        0x00U//涓流充电22H超时

//0x10E SW6306_CTRG_CHG7            充电配置7
#define SW6306_CHG7_MSK             0xCFU//充电配置7寄存器有效位
#define SW6306_CHG7_CHGEND_400M     0xC0U//充电截止电流400mA
#define SW6306_CHG7_CHGEND_300M     0x80U//充电截止电流300mA
#define SW6306_CHG7_CHGEND_100M     0x40U//充电截止电流100mA
#define SW6306_CHG7_CHGEND_200M     0x00U//充电截止电流200mA
#define SW6306_CHG7_C_3A3           0x0FU//C口5V充电电流3.3A
#define SW6306_CHG7_C_3A2           0x0EU//C口5V充电电流3.2A
#define SW6306_CHG7_C_3A1           0x0DU//C口5V充电电流3.1A
#define SW6306_CHG7_C_2A9           0x0CU//C口5V充电电流2.9A
#define SW6306_CHG7_C_2A8           0x0BU//C口5V充电电流2.8A
#define SW6306_CHG7_C_2A7           0x0AU//C口5V充电电流2.7A
#define SW6306_CHG7_C_2A6           0x09U//C口5V充电电流2.6A
#define SW6306_CHG7_C_2A5           0x08U//C口5V充电电流2.5A
#define SW6306_CHG7_C_2A4           0x07U//C口5V充电电流2.4A
#define SW6306_CHG7_C_2A3           0x06U//C口5V充电电流2.3A
#define SW6306_CHG7_C_2A2           0x05U//C口5V充电电流2.2A
#define SW6306_CHG7_C_2A1           0x04U//C口5V充电电流2.1A
#define SW6306_CHG7_C_2A0           0x03U//C口5V充电电流2.0A
#define SW6306_CHG7_C_1A9           0x02U//C口5V充电电流1.9A
#define SW6306_CHG7_C_1A8           0x01U//C口5V充电电流1.8A
#define SW6306_CHG7_C_3A0           0x00U//C口5V充电电流3.0A

//0x10F SW6306_CTRG_CHG8            充电配置8
#define SW6306_CHG8_B_2A3           0xE0U//B口5V充电电流2.3A
#define SW6306_CHG8_B_2A2           0xC0U//B口5V充电电流2.2A
#define SW6306_CHG8_B_2A1           0xA0U//B口5V充电电流2.1A
#define SW6306_CHG8_B_1A9           0x80U//B口5V充电电流1.9A
#define SW6306_CHG8_B_1A8           0x60U//B口5V充电电流1.8A
#define SW6306_CHG8_B_1A7           0x40U//B口5V充电电流1.7A
#define SW6306_CHG8_B_1A6           0x20U//B口5V充电电流1.6A
#define SW6306_CHG8_B_2A0           0x00U//B口5V充电电流2.0A
#define SW6306_CHG8_NTCUTP_NULL     0x1CU//NTC低温保护不生效
#define SW6306_CHG8_NTCUTP_M10C     0x18U//NTC-10°C低温保护
#define SW6306_CHG8_NTCUTP_M5C      0x14U//NTC-5°C低温保护
#define SW6306_CHG8_NTCUTP_20C      0x10U//NTC20°C低温保护
#define SW6306_CHG8_NTCUTP_15C      0x0CU//NTC15°C低温保护
#define SW6306_CHG8_NTCUTP_10C      0x08U//NTC10°C低温保护
#define SW6306_CHG8_NTCUTP_5C       0x04U//NTC5°C低温保护
#define SW6306_CHG8_NTCUTP_0C       0x00U//NTC0°C低温保护
#define SW6306_CHG8_NTCOTP_65C      0x03U//NTC65°C高温保护
#define SW6306_CHG8_NTCOTP_60C      0x02U//NTC60°C高温保护
#define SW6306_CHG8_NTCOTP_55C      0x01U//NTC55°C高温保护
#define SW6306_CHG8_NTCOTP_45C      0x00U//NTC45°C高温保护

//0x110 SW6306_CTRG_CHG9            充电配置9
#define SW6306_CHG9_LIBAT           0x80U//触发62368保护时减小IBAT限流值
#define SW6306_CHG9_LPOWER          0x00U//触发62368保护时减小功率
#define SW6306_CHG9_UT_7DIV8        0x70U//触发62368低温保护时减小功率/电流为原来的7/8（输入功率）
#define SW6306_CHG9_UT_6DIV8        0x60U//触发62368低温保护时减小功率/电流为原来的6/8
#define SW6306_CHG9_UT_5DIV8        0x50U//触发62368低温保护时减小功率/电流为原来的5/8
#define SW6306_CHG9_UT_4DIV8        0x40U//触发62368低温保护时减小功率/电流为原来的4/8
#define SW6306_CHG9_UT_3DIV8        0x30U//触发62368低温保护时减小功率/电流为原来的3/8
#define SW6306_CHG9_UT_2DIV8        0x20U//触发62368低温保护时减小功率/电流为原来的2/8
#define SW6306_CHG9_UT_1DIV8        0x10U//触发62368低温保护时减小功率/电流为原来的1/8
#define SW6306_CHG9_UT_NODIV        0x00U//触发62368低温保护时不减小功率/电流
#define SW6306_CHG9_OT_3DIV4        0x0CU//触发62368高温保护时减小功率/电流为原来的3/4（输入功率）
#define SW6306_CHG9_OT_2DIV4        0x08U//触发62368高温保护时减小功率/电流为原来的2/4
#define SW6306_CHG9_OT_1DIV4        0x04U//触发62368高温保护时减小功率/电流为原来的1/4
#define SW6306_CHG9_OT_NODIV        0x00U//触发62368高温保护时不减小功率/电流
#define SW6306_CHG9_OT_3DIV4        0x0CU//触发62368高温保护时减小功率/电流为原来的3/4
#define SW6306_CHG9_TR_TMSK         0x03U//62368充电高温范围有效位
#define SW6306_CHG9_TR_20C          0x02U//62368充电高温范围20°C
#define SW6306_CHG9_TR_10C          0x01U//62368充电高温范围10°C
#define SW6306_CHG9_TR_0C           0x00U//62368充电高温范围0°C

//0x111 SW6306_CTRG_CHG10           充电配置10
#define SW6306_CHG10_NOCHG_62368    0x80U//禁止充电时的62368功能
#define SW6306_CHG10_VL_0V35        0x70U//62368高温保护触发时每节充电电压降低0.35V
#define SW6306_CHG10_VL_0V3         0x60U//62368高温保护触发时每节充电电压降低0.3V
#define SW6306_CHG10_VL_0V25        0x50U//62368高温保护触发时每节充电电压降低0.25V
#define SW6306_CHG10_VL_0V2         0x40U//62368高温保护触发时每节充电电压降低0.2V
#define SW6306_CHG10_VL_0V15        0x30U//62368高温保护触发时每节充电电压降低0.15V
#define SW6306_CHG10_VL_0V1         0x20U//62368高温保护触发时每节充电电压降低0.1V
#define SW6306_CHG10_VL_0V05        0x10U//62368高温保护触发时每节充电电压降低0.05V
#define SW6306_CHG10_VL_NONE        0x00U//62368高温保护触发时不降低每节充电电压
#define SW6306_CHG10_NODIS_62368    0x08U//禁止放电时的62368功能
#define SW6306_CHG10_TN_MSK         0x07U//62368充电常温上限有效位
#define SW6306_CHG10_TN_45C         0x06U//62368充电常温上限45°C（请设置为该参数，否则会触发低温限制）
#define SW6306_CHG10_TN_40C         0x05U//62368充电常温上限40°C
#define SW6306_CHG10_TN_35C         0x04U//62368充电常温上限35°C
#define SW6306_CHG10_TN_30C         0x03U//62368充电常温上限30°C
#define SW6306_CHG10_TN_25C         0x02U//62368充电常温上限25°C
#define SW6306_CHG10_TN_5C          0x01U//62368充电常温上限5°C
#define SW6306_CHG10_TN_20C         0x00U//62368充电常温上限20°C

//0x112 SW6306_CTRG_CHG11           充电配置11
#define SW6306_CHG11_UT_0D7         0xC0U//触发62368低温保护时减小功率/电流为原来的0.7（输出功率）
#define SW6306_CHG11_UT_0D5         0x80U//触发62368低温保护时减小功率/电流为原来的0.5
#define SW6306_CHG11_UT_0D2         0x40U//触发62368低温保护时减小功率/电流为原来的0.2
#define SW6306_CHG11_UT_NONE        0x00U//触发62368低温保护时不减小功率/电流
#define SW6306_CHG11_UT_FIX         0x20U//触发62368低温保护时输出固定电压电流
#define SW6306_CHG11_UT_LOW         0x00U//触发62368低温保护时减小输出功率/电流
#define SW6306_CHG11_UT_FIX5V1A5    0x10U//触发62368低温保护时输出固定5V1.5A
#define SW6306_CHG11_UT_FIX5V3A     0x00U//触发62368低温保护时输出固定5V3A
#define SW6306_CHG11_TR_TMSK        0x03U//62368放电常温范围有效位
#define SW6306_CHG11_TR_NTC         0x02U//62368放电常温范围跟随NTC放电正常范围
#define SW6306_CHG11_TR_60C         0x01U//62368放电常温范围60°C
#define SW6306_CHG11_TR_45C         0x00U//62368放电常温范围45°C

//0x113 SW6306_CTRG_CHG12           充电配置12
#define SW6306_CHG12_MSK            0xF0U//充电配置12寄存器有效位
#define SW6306_CHG12_NOCT           0x80U//禁止充电恒温环
#define SW6306_CHG12_ENCT           0x00U//使能充电恒温环
#define SW6306_CHG12_CT_130C        0x70U//芯片充电恒温环阈值130°C
#define SW6306_CHG12_CT_120C        0x60U//芯片充电恒温环阈值120°C
#define SW6306_CHG12_CT_110C        0x50U//芯片充电恒温环阈值110°C
#define SW6306_CHG12_CT_100C        0x40U//芯片充电恒温环阈值100°C
#define SW6306_CHG12_CT_90C         0x30U//芯片充电恒温环阈值90°C
#define SW6306_CHG12_CT_80C         0x20U//芯片充电恒温环阈值80°C
#define SW6306_CHG12_CT_7C          0x10U//芯片充电恒温环阈值70°C
#define SW6306_CHG12_CT_6C          0x00U//芯片充电恒温环阈值60°C

//0x114 SW6306_CTRG_DCDC0           BUCK-BOOST变换器设置0
#define SW6306_DCDC0_F500K          0xC0U//开关频率500K
#define SW6306_DCDC0_F400K          0x80U//开关频率400K
#define SW6306_DCDC0_F200K          0x40U//开关频率200K
#define SW6306_DCDC0_F300K          0x00U//开关频率300K
#define SW6306_DCDC0_18A            0x30U//充放电共用峰值限流值18A
#define SW6306_DCDC0_16A            0x20U//充放电共用峰值限流值16A
#define SW6306_DCDC0_14A            0x10U//充放电共用峰值限流值14A
#define SW6306_DCDC0_12A            0x00U//充放电共用峰值限流值12A
#define SW6306_DCDC0_150C           0x0CU//芯片150°C过温保护
#define SW6306_DCDC0_140C           0x08U//芯片140°C过温保护
#define SW6306_DCDC0_130C           0x04U//芯片130°C过温保护
#define SW6306_DCDC0_120C           0x00U//芯片120°C过温保护

//0x115 SW6306_CTRG_DCDC1           BUCK-BOOST变换器设置1
#define SW6306_DCDC1_MSK            0xC0U//BUCK-BOOST变换器设置1寄存器有效位
#define SW6306_DCDC1_10R            0xC0U//BUS端下管Rdson 10mR
#define SW6306_DCDC1_7R5            0x80U//BUS端下管Rdson 7.5mR
#define SW6306_DCDC1_5R             0x40U//BUS端下管Rdson 5mR
#define SW6306_DCDC1_2R5            0x00U//BUS端下管Rdson 2.5mR

//0x116 SW6306_CTRG_DCDC2           BUCK-BOOST变换器设置2
#define SW6306_DCDC2_MSK            0x5EU//BUCK-BOOST变换器设置2寄存器有效位
#define SW6306_DCDC2_FPWM           0x40U//轻载强制运行在PWM模式
#define SW6306_DCDC2_NONTC          0x10U//禁止NTC保护
#define SW6306_DCDC2_TEMP_REGSET    0x08U//充放电温度由寄存器控制
#define SW6306_DCDC2_62368_REGSET   0x04U//充放电62368功能由寄存器控制
#define SW6306_DCDC2_CT_REGSET      0x02U//充放电恒温环由寄存器控制

//0x117 SW6306_CTRG_INDET0          插拔检测设置0
#define SW6306_INDET0_MSK           0xC0U//插拔检测设置0寄存器有效位
#define SW6306_INDET0_NOA1VBUS      0x80U//禁止A1口VBUS接入检测
#define SW6306_INDET0_ENA1VBUS      0x00U//使能A1口VBUS接入检测
#define SW6306_INDET0_NOA2VBUS      0x40U//禁止A2口VBUS接入检测
#define SW6306_INDET0_ENA2VBUS      0x00U//使能A2口VBUS接入检测

//0x118 SW6306_CTRG_INDET1          插拔检测设置1
#define SW6306_INDET1_NOA1NL        0x20U//禁止A1口空载
#define SW6306_INDET1_ENA1NL        0x00U//使能A1口空载
#define SW6306_INDET1_NOA2NL        0x10U//禁止A2口空载
#define SW6306_INDET1_ENA2NL        0x00U//使能A2口空载
#define SW6306_INDET1_NOC1NL        0x08U//禁止C1口空载
#define SW6306_INDET1_ENC1NL        0x00U//使能C1口空载
#define SW6306_INDET1_NOC2NL        0x04U//禁止C2口空载
#define SW6306_INDET1_ENC2NL        0x00U//使能C2口空载

//0x119 SW6306_CTRG_INDET2          插拔检测设置2
#define SW6306_INDET2_MPNL_64S      0xC0U//多口空载时间64s
#define SW6306_INDET2_MPNL_16S      0x80U//多口空载时间16s
#define SW6306_INDET2_MPNL_8S       0x40U//多口空载时间8s
#define SW6306_INDET2_MPNL_32S      0x00U//多口空载时间32s
#define SW6306_INDET2_WLNL_64S      0x30U//无线充空载时间64s
#define SW6306_INDET2_WLNL_32S      0x20U//无线充空载时间32s
#define SW6306_INDET2_WLNL_16S      0x10U//无线充空载时间16s
#define SW6306_INDET2_WLNL_128S     0x00U//无线充空载时间128s
#define SW6306_INDET2_HVHALFNL      0x08U//高压时拔出电流值减半
#define SW6306_INDET2_HVNORMNL      0x00U//高压时拔出电流值不减半
#define SW6306_INDET2_SPNL_MSK      0x07U//单口空载时间有效位
#define SW6306_INDET2_SPNL_128S     0x04U//单口空载时间128s
#define SW6306_INDET2_SPNL_64S      0x03U//单口空载时间64s
#define SW6306_INDET2_SPNL_16S      0x02U//单口空载时间16s
#define SW6306_INDET2_SPNL_8S       0x01U//单口空载时间8s
#define SW6306_INDET2_SPNL_32S      0x00U//单口空载时间32s

#define SW6306_CTRG_INDET3          0x11AU//插拔检测设置3
#define SW6306_CTRG_INDET4          0x11BU//插拔检测设置4
#define SW6306_CTRG_INDET5          0x11DU//插拔检测设置5

#define SW6306_CTRG_MODEWLSS        0x11EU//无线充场景配置

#define SW6306_CTRG_PORTQC          0x11FU//端口快充配置
#define SW6306_CTRG_VCHG            0x120U//输入电压配置

//0x122 SW6306_CTRG_P_DPDM0         DPDM协议设置0
#define SW6306_P_DPDM0_MSK          0xF0U//DPDM协议设置0寄存器有效位
#define SW6306_P_DPDM0_PSYS         0xC0U//高压快充协议功率跟随系统设置
#define SW6306_P_DPDM0_45W          0x80U//高压快充协议功率45W
#define SW6306_P_DPDM0_30W          0x40U//高压快充协议功率30W
#define SW6306_P_DPDM0_18W          0x10U//高压快充协议功率18W
#define SW6306_P_DPDM0_NOAPPLE      0x20U//禁止多口输出时的苹果模式
#define SW6306_P_DPDM0_NO1V2        0x10U//禁止三星1.2V模式

//0x123 SW6306_CTRG_P_DPDM1         DPDM协议设置1
#define SW6306_DPDM1_QC3P_MSK       0xC0U//QC3+协议最大功率有效位
#define SW6306_DPDM1_QC3P_45W       0xC0U//QC3+协议最大功率45W
#define SW6306_DPDM1_QC3P_40W       0x80U//QC3+协议最大功率40W
#define SW6306_DPDM1_QC3P_27W       0x40U//QC3+协议最大功率27W
#define SW6306_DPDM1_QC3P_18W       0x00U//QC3+协议最大功率18W
#define SW6306_DPDM1_QC2_MSK        0x30U//QC2协议最大电压有效位
#define SW6306_DPDM1_QC2_20V        0x20U//QC2协议最大电压20V
#define SW6306_DPDM1_QC2_9V         0x10U//QC2协议最大电压9V
#define SW6306_DPDM1_QC2_12V        0x00U//QC2协议最大电压12V
#define SW6306_DPDM1_QC3_MSK        0x0CU//QC3协议最大电压有效位
#define SW6306_DPDM1_QC3_9V         0x08U//QC3协议最大电压9V
#define SW6306_DPDM1_QC3_12V        0x04U//QC3协议最大电压12V
#define SW6306_DPDM1_QC3_20V        0x00U//QC3协议最大电压20V
#define SW6306_DPDM1_AFC_9V         0x02U//AFC协议最大电压9V
#define SW6306_DPDM1_AFC_12V        0x00U//AFC协议最大电压12V
#define SW6306_DPDM1_FCP_9V         0x01U//FCP协议最大电压9V
#define SW6306_DPDM1_FCP_12V        0x00U//FCP协议最大电压12V

//0x124 SW6306_CTRG_P_DPDM2         DPDM协议设置2
#define SW6306_DPDM2_MSK            0xC0U//DPDM协议设置2有效位
#define SW6306_DPDM2_SFCP_12V       0x80U//SFCP协议最大电压12V
#define SW6306_DPDM2_SFCP_9V        0x00U//SFCP协议最大电压9V
#define SW6306_DPDM2_PE_12V         0x40U//PE协议最大电压12V
#define SW6306_DPDM2_PE_9V          0x00U//PE协议最大电压9V

//0x12B SW6306_CTRG_P_DPDM4         DPDM协议设置4
#define SW6306_P_DPDM4_MSK          0x38U//DPDM协议设置4有效位
#define SW6306_P_DPDM4_VOOC1        0x20U//VOOC 1.0协议使能
#define SW6306_P_DPDM4_VOOC4        0x10U//VOOC 4.0协议使能
#define SW6306_P_DPDM4_SVOOC        0x08U//Super VOOC协议使能

//0x12C SW6306_CTRG_P_DPDM5         DPDM协议设置5
#define SW6306_P_DPDM5_MSK          0x11U//DPDM协议设置5有效位
#define SW6306_P_DPDM5_VOOC         0x10U//VOOC充电使能
#define SW6306_P_DPDM5_SDP2A        0x01U//SDP抽取2A电流

//0x132 SW6306_CTRG_TYPEC1          C1 Type C设置
#define SW6306_TYPEC_VCONN_NOOCP    0x80U//禁止VCONN引脚过流异常检测
#define SW6306_TYPEC1_MSK           0x03U//C1 Type C设置寄存器有效位
#define SW6306_TYPEC1_SNK           0x02U//Type C2仅为sink
#define SW6306_TYPEC1_SRC           0x01U//Type C2仅为source
#define SW6306_TYPEC1_DRP           0x00U//Type C2为有try.SRC功能的DRP口

//0x14B SW6306_CTRG_TYPEC2          C2 Type C设置
#define SW6306_TYPEC2_MSK           0xC0U//C2 Type C设置寄存器有效位
#define SW6306_TYPEC2_SNK           0x80U//Type C2仅为sink
#define SW6306_TYPEC2_SRC           0x40U//Type C2仅为source
#define SW6306_TYPEC2_DRP           0x00U//Type C2为有try.SRC功能的DRP口

//0x14D SW6306_CTRG_DISPLAY         显示设置
#define SW6306_DISPLAY_MSK          0x0EU//显示设置寄存器有效位
#define SW6306_DISPLAY_8_20M        0x0CU//LED/数码管驱动电流8/20mA
#define SW6306_DISPLAY_2_5M         0x08U//LED/数码管驱动电流2/5mA
#define SW6306_DISPLAY_6_15M        0x04U//LED/数码管驱动电流6/15mA
#define SW6306_DISPLAY_4_10M        0x00U//LED/数码管驱动电流4/10mA
#define SW6306_DISPLAY_5SDELAY      0x02U//轻载5s后关闭输出

//0x14E SW6306_CTRG_GAUGE0          库仑计设置0
#define SW6306_GAUGE0_MSK           0x90U//库仑计设置0寄存器有效位
#define SW6306_GAUGE0_RDCEN         0x80U//Rdc计算使能
#define SW6306_GAUGE0_LEARNEN       0x10U//容量学习使能

//0x14F SW6306_CTRG_GAUGE1          库仑计设置1
#define SW6306_GAUGE1_99WAIT        0x20U//充至99%后等待十分钟才充电至100%

//0x150 SW6306_CTRG_KEY0            按键设置0
#define SW6306_KEY0_SPEXIT          0x40U//短按键退出蓝牙模式或关闭WLED
#define SW6306_KEY0_FILTER          0x08U//快充场景防误触发使能
#define SW6306_KEY0_RDPORT          0x04U//短按键打开有Rd电阻的口
#define SW6306_KEY0_NOVBUS          0x02U//短按键打开VBUS建立不起来的口
#define SW6306_KEY0_REGSET          0x01U//短按键功能由寄存器决定

//0x151 SW6306_CTRG_KEY1            按键设置1
#define SW6306_KEY1_DISPLAY         0x50U//短按键打开灯显与已经接入的输出口
#define SW6306_KEY1_RDPORT          0x40U//短按键打开有Rd电阻的口
#define SW6306_KEY1_NOVBUS          0x30U//短按键打开VBUS建立不起来的口
#define SW6306_KEY1_A1A2            0x20U//短按键打开A1与A2口
#define SW6306_KEY1_A2              0x10U//短按键打开A2口
#define SW6306_KEY1_A1              0x00U//短按键打开A1口
#define SW6306_KEY1_LPOFF           0x0CU//长按关闭下游口
#define SW6306_KEY1_LPBLU           0x08U//长按打开小电流模式
#define SW6306_KEY1_LPWLED          0x04U//长按打开WLED
#define SW6306_KEY1_LPAUTO          0x00U//长按根据识别电阻，进入小电流模式/打开WLED
#define SW6306_KEY1_DCAUTO          0x03U//双击根据识别电阻，进入小电流模式/打开WLED
#define SW6306_KEY1_DCWLED          0x02U//双击打开WLED
#define SW6306_KEY1_DCBLU           0x01U//双击打开小电流模式
#define SW6306_KEY1_DCFF            0x00U//双击关闭下游口

//0x153 SW6306_CTRG_IPPS            PPS电流偏移设置
#define SW6306_IPPS_MSK             0x30U//PPS电流偏移设置寄存器有效位
#define SW6306_IPPS_300M            0x30U//PPS限流偏移300mA
#define SW6306_IPPS_200M            0x20U//PPS限流偏移200mA
#define SW6306_IPPS_100M            0x10U//PPS限流偏移100mA
#define SW6306_IPPS_NONE            0x00U//PPS限流无偏移

//0x154 SW6306_CTRG_FAULT1          异常/保护使能1
#define SW6306_FAULT1_NOVBUSOVP     0x08U//禁止放电时VBUS慢速过压保护
#define SW6306_FAULT1_NODPDMOVP     0x04U//禁止放电时DPDM 5.5V过压保护
#define SW6306_FAULT1_NOCCOVP       0x02U//禁止放电时CC1/2 5.5V过压保护

//0x155 SW6306_CTRG_FAULT2          异常/保护使能2
#define SW6306_FAULT2_62368_NOUTP   0x80U//禁止放电62368低温保护
#define SW6306_FAULT2_NOCCFLAG      0x40U//禁止CC/CV切换时发送cc flag

//0x156 SW6306_CTRG_FAULT3          异常/保护使能3
#define SW6306_FAULT3_NTC_NOOTP     0x04U//禁止充电NTC过温保护
#define SW6306_FAULT3_62368_NOOTP   0x02U//禁止充电62368过温保护
#define SW6306_FAULT3_62368_NOUTP   0x01U//禁止充电62368低温保护

/*****************************SW6306配置结构体*********************************/
//中断配置结构体
//放电配置结构体
//充电配置结构体
//升降压配置结构体
//端口配置结构体

//基础操作
uint8_t SW6306_ByteWrite(uint8_t reg, uint8_t data);
uint8_t SW6306_ByteRead(uint8_t reg, uint8_t *data);
uint8_t SW6306_RegsetSwitch(uint16_t regset);
uint8_t SW6306_ByteModify(uint8_t reg, uint8_t mask, uint8_t data);
uint8_t SW6306_ADCRead(uint8_t ch, uint16_t *pData);//读取ADC数据，未对寄存器组作检查
//ADC数据相关操作
uint8_t SW6306_ADCLoad(void);                   //读取全部ADC数据并更新镜像寄存器
uint16_t SW6306_ReadVBUS(void);                 //读取BUS电压
uint16_t SW6306_ReadIBUS(void);                 //读取BUS电流
uint16_t SW6306_ReadVBAT(void);                 //读取BAT电压
uint16_t SW6306_ReadIBAT(void);                 //读取BAT电流
int16_t SW6306_ReadTNTC(void);                  //读取NTC温度（结果为5的倍数，并不准确，不推荐使用）
float SW6306_ReadTCHIP(void);                   //读取芯片温度
float SW6306_ReadVNTC(void);                    //读取NTC电压
//状态相关操作
uint8_t SW6306_StatusLoad(void);                //将SW6306的各种状态读取到镜像寄存器(0x12,0x14,0x15,0x18,0x1A,0x2A,0x2B,0x2C)
uint8_t SW6306_IsLowCurrentMode(void);          //SW6306是否处于小电流模式
uint8_t SW6306_IsMPPTCharging(void);            //SW6306是否处于MPPT充电模式
uint8_t SW6306_IsCharging(void);                //SW6306是否正在充电
uint8_t SW6306_IsDischarging(void);             //SW6306是否正在放电
uint8_t SW6306_IsFullCharged(void);             //SW6306是否充满
uint8_t SW6306_IsBatteryDepleted(void);         //SW6306电池是否耗尽
uint8_t SW6306_IsCapacityLearned(void);         //是否已完成电量学习
uint8_t SW6306_IsErrorinCharging(void);         //充电是否出现异常
uint8_t SW6306_IsErrorinDischarging(void);      //放电是否出现异常
uint8_t SW6306_IsKeyEvent(void);                //是否触发了按键事件
uint8_t SW6306_IsSceneChanged(void);            //是否发生场景变化
uint8_t SW6306_IsOverHeated(void);              //是否发生过温异常
float SW6306_TNTC_Calc(void);                   //计算并返回NTC温度
//端口状态相关操作
uint8_t SW6306_PortStatusLoad(void);            //更新端口状态镜像寄存器(0x13,0x18,0x19,0x1C,0x1D)
uint8_t SW6306_IsPortC1ON(void);                //读取C1口通路是否打开
uint8_t SW6306_IsPortC2ON(void);                //读取C2口通路是否打开
uint8_t SW6306_IsPortA1ON(void);                //读取A1口通路是否打开
uint8_t SW6306_IsPortA2ON(void);                //读取A2口通路是否打开
//功率与协议相关操作
uint8_t SW6306_PowerLoad(void);                 //更新功率状态镜像寄存器(0x0E,0x0F,0x10,0x11,0x1C,0x45,0x4F,0x51,0x52)
uint16_t SW6306_ReadIPortLimit(void);           //读取充电时端口限流实时值（单位：mA）
uint16_t SW6306_ReadIBattLimit(void);           //读取充电时电池限流实时值（单位：mA）
uint8_t SW6306_ReadMaxOutputPower(void);        //读取最大输出功率（单位：W）
uint8_t SW6306_ReadMaxInputPower(void);         //读取最大输入功率（单位：W）
//容量与库仑计相关操作
uint8_t SW6306_CapacityLoad(void);              //更新容量与库仑计镜像寄存器(0x86~0x8A,0x99,0xA2)
uint8_t SW6306_ReadCapacity(void);              //读取SW6306显示电量
float SW6306_ReadMaxGuageCap(void);             //读取库仑计最大容量（单位：mAh）
float SW6306_ReadPresentGuageCap(void);         //读取库仑计当前容量（单位：mAh）

uint8_t SW6306_ForceOff(void);                  //强制关闭放电并休眠
uint8_t SW6306_Unlock(void);                    //解除低功耗，解锁SW6306的寄存器写入
uint8_t SW6306_Click(void);                     //触发单击事件
uint8_t SW6306_LPSet(void);                     //打开低功耗

uint8_t SW6306_WLEDSet(uint8_t wledstatus);     //控制WLED脚电平
uint8_t SW6306_IO1Set(uint8_t io1status);       //控制IO1脚电平

//强制控制BUS电压
//强制控制BUS限流
//强制控制BAT电压
//强制控制BAT限流

uint8_t SW6306_Init(void);                      //初始化，最好系统上电后立刻执行
uint8_t SW6306_IsInitialized(void);             //检测SW6306是否已初始化过，须在SW6306_PowerLoad()后执行
#ifdef __cplusplus
}
#endif

#endif
/*注释与经验：
0. BUS端指H桥两端中面向接口的那一端；BAT端指H桥两端中面向电池的那一端；
1. SW6306 ADC通道设置后可以直接读其值，不必等待转换时间
2. SW6306存在超过256个寄存器，需要在高低地址间切换且写入前需解锁
3. 充放电功率设置各有两个寄存器，其中0x100寄存器不使用，使用0x40地址的寄存器设置
*/