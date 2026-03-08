# SW6306移动电源程序
## 功能说明：
3S1P 21700移动电源的主要源程序

支持所有端口的插入检测，A口无负载时延迟一段时间关断  
定时读取SW6306信息并通过串口以115200波特率上传  
空闲状态下10s单片机会自动休眠，但在LED开启或充放电状态下则保持开启

休眠状态按下数码管会亮起5s，若无负载接入自动关闭
插入充电器/负载时数码管会自动亮起，并保持开启；但单A口负载低于50mA时一段时间后会自动关闭
电池亏电时数码管显示0%，并不断闪烁；出现异常（例如两个C口连接接在一起）数码管显示188%并闪烁一段时间后关闭
放电状态，且LED照明未激活时长按按键，可以强制关闭输出并使数码管关闭
数码管关闭后持续按键越15s，可以触发单片机复位，此时数码管重新亮起。该功能可用于SW6306出现异常且不便打开外壳时重新进行初始化
任何状态下快速双击（按下-松开-按下-松开）可以开关LED照明
LED照明打开时长按可以调光（调亮/调暗），松开按键再次长按可以更换调光方向；单击不起作用
LED照明打开时长按关闭输出与超长按复位功能不生效，需要先双击退出LED照明模式

相对原版芯片解锁了20V QC2协议与11V5A PPS  
考虑到三星50G的电流能力将充放电功率分别限制在 60 / 90 W
若使用高性能电池（如2170LA、新能安 全极耳等）可在sw6306.h文件中修改最大功率

多任务调度基于[Protothread](https://dunkels.com/adam/pt/)，进行了部分封装
## 软硬件环境：
### 硬件主控：
合宙 Air001（换皮PUYA PY32F002AF15P，利用了一些手册上未提及的配置）
### 硬件连接：
接线参考：
```
SW6306              AIR001              DAPLINK
TMOD/I2C    ->      GND         ->      GND
PISET/SDA   ->      PF0  
POSET/SCL   ->      PF1  
NMOD/IRQ    ->      PA6
                    PA2(TX)     ->      RX
                    PA3(RX)     ->      TX
                    PA13        ->      SWDIO
                    PA14        ->      SWCLK
                    PF2/NRST    ->      RST
```
**电池：21700三串**
### 开发环境：
* ARM KEIL 礦ision5 V5.33  
* ARM Compiler V6.15
* 软件包：Puya.PY32F0xx_DFP.1.1.0.pack
* PY32F0xx LL库
### 文件说明：
Project.uvprojx所在根目录：
* main.c：各个任务的主体部分
* hardware_config.h：系统时钟与硬件连接宏定义，供Drivers内的各bsp文件使用
* software_config.h：波特率与软件时基宏定义，供main.c使用
* py32f0xx_it.c：Systick中断中自增系统时基

Drivers（包括所属目录）：
* SW6306.c .h：SW6306库本体，提供各种操作方法 **（更改电池参数等重要配置请修改C文件中初始化部分）**
* bsp_py32f030x6.c .h：BSP驱动总领文件，负责将所有BSP驱动文件提供的初始化方法打包为SysInit()函数
* py32f0xx_bsp_exti.c .h：外部中断文件，负责初始化IRQ与按键的中断
* py32f0xx_bsp_gpio.c .h：GPIO初始化
* py32f0xx_bsp_i2c.c .h：I2C库，提供了中断模式下I2C外设的异步读写
* py32f0xx_bsp_tim.c .h：定时器与PWM通道初始化，提供了调光函数的具体实现
* py32f0xx_bsp_pwr.c .h：睡眠与低功耗特性初始化
* py32f0xx_bsp_rcc.c .h：时钟系统与Systick初始化
* py32f0xx_bsp_usart.c .h：USART库，提供了中断方式实现的UART带缓冲区收发
* 剩余为PY32 LL库文件

CoroOS（包括所属目录）：
* coroOS.h：ProtoThread库的封装
* pt.h：ProtoThread库本体
* lc.h：C语言switch实现函数可重入的方法

### 固件下载过程：
方法一：直接使用对应或更高的版本打开Project.uvprojx，F7编译+F8下载  
方法二：在Objects文件夹内找到Project.hex，使用下载器对应的软件打开下载
## 问题与缺陷：
* 未实现SW6306的异常状态处理，仅仅是重新复位初始化
* 