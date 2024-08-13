#include "py32f0xx_bsp_usart.h"

void BSP_USART_Config(uint32_t baudRate)
{
    DEBUG_USART_CLK_ENABLE();

    /* USART Init */
    LL_USART_SetBaudRate(DEBUG_USART, SystemCoreClock, LL_USART_OVERSAMPLING_16, baudRate);
    LL_USART_SetDataWidth(DEBUG_USART, LL_USART_DATAWIDTH_8B);
    LL_USART_SetStopBitsLength(DEBUG_USART, LL_USART_STOPBITS_1);
    LL_USART_SetParity(DEBUG_USART, LL_USART_PARITY_NONE);
    LL_USART_SetHWFlowCtrl(DEBUG_USART, LL_USART_HWCONTROL_NONE);
    LL_USART_SetTransferDirection(DEBUG_USART, LL_USART_DIRECTION_TX_RX);
    LL_USART_Enable(DEBUG_USART);
    LL_USART_ClearFlag_TC(DEBUG_USART);

    /**USART GPIO Configuration
    PA2     ------> USART1_TX
    PA3     ------> USART1_RX
    */
    DEBUG_USART_RX_GPIO_CLK_ENABLE();
    DEBUG_USART_TX_GPIO_CLK_ENABLE();

    LL_GPIO_SetPinMode(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetAFPin_0_7(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, DEBUG_USART_TX_AF);

    LL_GPIO_SetPinMode(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetAFPin_0_7(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, DEBUG_USART_RX_AF);
    #ifdef USART_ENABLEIT
    LL_USART_EnableIT_PE(DEBUG_USART);
    LL_USART_EnableIT_ERROR(DEBUG_USART);
    LL_USART_EnableIT_RXNE(DEBUG_USART);
    NVIC_SetPriority(DEBUG_USART_IRQ, 1);
    NVIC_EnableIRQ(DEBUG_USART_IRQ);
    #endif
}

#ifdef USART_ENABLEIT
/*通过实现一个双指针的环形缓冲区来传递数据，收发操作方法相同
/rd指针跟随wr运动，在wr地址-1处停下，向缓冲区填数据时，wr自增，直到上限时归零
/wr指针操作遵循先操作数据，之后马上移动指针的顺序，rd指针相反
*/
/*****************************发送部分*****************************************/
#if USART_TX_BUFFER_SIZE <= 256
    static uint8_t tx_wr_index = 0,tx_rd_index = USART_TX_BUFFER_SIZE-1;
#else
    static uint16_t tx_wr_index = 0,tx_rd_index = USART_TX_BUFFER_SIZE-1;
#endif
static uint8_t USART_tx_buffer[USART_TX_BUFFER_SIZE];//缓冲区数组定义

/*
/串口发送区填充函数，也负责在发送区空闲时启动第一个数据的发送，溢出的数据将被丢弃
/参数：*UT_Data:发送数据的地址；UT_len:发送数据的长度
/返回值：1代表溢出，0代表正常填充
*/
#if DEBUG_USART_TX_BUFFER_SIZE <= 256
uint8_t USART_bufsend(uint8_t *UT_Data,uint8_t UT_len)
{   
    uint8_t UT_i;
#else
uint8_t USART_bufsend(uint8_t *UT_Data,uint16_t UT_len)
{   
    uint16_t UT_i;
#endif
    for(UT_i = 0;UT_i < UT_len;UT_i++)//循环填充
    {
        if(UT_Data[UT_i] == 0x00) break;//遇到字符串结尾便退出
        if(tx_wr_index == USART_TX_BUFFER_SIZE-1)//已经到环形队列尾部
        {
            if(tx_rd_index) 
            {
                USART_tx_buffer[tx_wr_index] = UT_Data[UT_i];
                tx_wr_index = 0;
            }
            else return 1;
        }
        else
        {
            if(tx_wr_index+1 == tx_rd_index) return 1;
            else
            {   
                USART_tx_buffer[tx_wr_index] = UT_Data[UT_i];
                tx_wr_index++;
            }
        }
    }    
    if(UT_len && (LL_USART_IsEnabledIT_TXE(DEBUG_USART) == 0))//如果有数据需要发送且USART未处于发送状态
    {//发送状态由TXE中断是否使能表示
        if(tx_rd_index == USART_TX_BUFFER_SIZE-1) tx_rd_index = 0;//rd指针操作
        else tx_rd_index++;
        LL_USART_TransmitData8(DEBUG_USART,USART_tx_buffer[tx_rd_index]);
        LL_USART_EnableIT_TXE(DEBUG_USART);
    }   
    return 0;
}

/*
/串口发送区填充函数（单字节），也负责在发送区空闲时启动第一个数据的发送，溢出的数据将被丢弃
/参数：uint8_t UT_Data：要发送的一字节数据
/返回值：1代表溢出，0代表正常填充
*/
uint8_t USART_send_a_byte(uint8_t UT_Data)
{    
    if(tx_wr_index == USART_TX_BUFFER_SIZE-1)
    {
        if(tx_rd_index) 
        {
            USART_tx_buffer[tx_wr_index] = UT_Data;
            tx_wr_index = 0;
        }
        else return 1;
    }
    else
    {
        if(tx_wr_index+1 == tx_rd_index) return 1;
        else
        {   
            USART_tx_buffer[tx_wr_index] = UT_Data;
            tx_wr_index++;
        }
    }
    if(LL_USART_IsEnabledIT_TXE(DEBUG_USART) == 0)//如果有数据需要发送且USART未处于发送状态
    {//发送状态由TXE中断是否使能表示
        if(tx_rd_index == USART_TX_BUFFER_SIZE-1) tx_rd_index = 0;//rd指针操作
        else tx_rd_index++;
        LL_USART_TransmitData8(DEBUG_USART,USART_tx_buffer[tx_rd_index]);
        LL_USART_EnableIT_TXE(DEBUG_USART);
    }   
    return 0;
}

/*******************************接收部分***************************************/
#if USART_RX_BUFFER_SIZE <= 256
    static uint8_t rx_wr_index = 0,rx_rd_index = USART_RX_BUFFER_SIZE-1;
#else
    static uint16_t rx_wr_index = 0,rx_rd_index = USART_RX_BUFFER_SIZE-1;
#endif
    
uint8_t rx_buf_ovf = 0;

static uint8_t USART_rx_buffer[USART_RX_BUFFER_SIZE];//缓冲区数组定义

/*接收缓冲区查空函数，返回缓冲区内有效数据个数
/通过wr-rd得出
*/
#if USART_RX_BUFFER_SIZE <= 256
uint8_t USART_getvalidnum()
#else
uint16_t USART_getvalidnum()
#endif
{
    if(rx_wr_index > rx_rd_index) return rx_wr_index-rx_rd_index-1;
    else return USART_RX_BUFFER_SIZE-1-rx_rd_index+rx_wr_index;
}

#define UART_READ_DEFAULT 0xFF
/*单字节串口接收缓冲区读取函数
/参数：无
/返回值：从缓冲区中读出的一字节数据
/备注：不应在缓冲区为空时执行该函数，否则返回一个默认值
*/
uint8_t USART_read_a_byte()
{
    //uint8_t UT1_rx;
    if(rx_rd_index == USART_RX_BUFFER_SIZE-1)//读到数组末尾
    {
        if(rx_wr_index == 0) return UART_READ_DEFAULT;//没有数据了
        else
        {
            rx_rd_index = 0;
            return USART_rx_buffer[0];
        }
    }
    else
    {
        if(rx_wr_index == rx_rd_index+1) return UART_READ_DEFAULT;//没有数据了
        else
        {
            rx_rd_index++;
            return USART_rx_buffer[rx_rd_index];
        }
    }
}

/*
/多字节串口接收缓冲区读取函数，将缓冲区指定个数的数据写入上层函数提供的数组内
/不对上游数组作越界检查
/参数：*pdata:目标数组的地址; num:读取数据的个数
/返回值：是否发生过量读取，溢出时返回1
*/
#if DEBUG_USART_RX_BUFFER_SIZE <= 256
uint8_t USART_bufread(uint8_t *UT_Data,uint8_t UT_len)
{   
    uint8_t UT_i;
#else
uint8_t USART_bufread(uint8_t *UT_Data,uint16_t UT_len)
{   
    uint16_t UT_i;
#endif
    for(UT_i = 0;UT_i < UT_len;UT_len++)
    {
        if(rx_rd_index == USART_RX_BUFFER_SIZE-1)//读到数组末尾
        {
            if(rx_wr_index == 0) return 1;//没有数据了
            else 
            {
                rx_rd_index = 0;
                UT_Data[UT_i] = USART_rx_buffer[rx_rd_index];
            }
        }
        else
        {
            if(rx_wr_index == rx_rd_index+1) return 1;//没有数据了
            else
            {
                rx_rd_index++;
                UT_Data[UT_i] = USART_rx_buffer[rx_rd_index];
            }
        }
    }
        return 0;
}

void DEBUG_USART_IRQHandler()
{
    if(LL_USART_IsActiveFlag_TXE(DEBUG_USART))
    {/*串口发送中断ISR，用于在发送缓冲区未空时连续将缓冲区数据填充到UART_DR中去
    /串口发送寄存器空中断使能位也标识着缓冲区内的数据是否发送完了
    /不及时清零串口发送寄存器空中断使能位，会导致重复进入中断处理函数*/
        if(tx_wr_index - tx_rd_index == 1) LL_USART_DisableIT_TXE(DEBUG_USART);
        else if((tx_wr_index == 0) && (tx_rd_index == USART_TX_BUFFER_SIZE-1)) LL_USART_DisableIT_TXE(DEBUG_USART);//数据发完了
        else
        {
            if(tx_rd_index == USART_TX_BUFFER_SIZE-1) tx_rd_index = 0;
            else tx_rd_index++;
            LL_USART_TransmitData8(DEBUG_USART,USART_tx_buffer[tx_rd_index]);
        }
    }
    if(LL_USART_IsActiveFlag_RXNE(DEBUG_USART))
    {//串口接收中断，负责实时将收到数据填充到缓冲区，溢出的数据被丢弃
        if(rx_wr_index == USART_RX_BUFFER_SIZE-1)//写到数组末尾了
        {
            if(rx_rd_index == 0)//缓冲区塞满了
            {
                rx_buf_ovf = 1;
            }
            else
            {
                USART_rx_buffer[rx_wr_index] = LL_USART_ReceiveData8(DEBUG_USART);
                rx_wr_index = 0;
            }
        }
        else
        {
            if(rx_rd_index - rx_wr_index == 1)//缓冲区塞满了
            {
                rx_buf_ovf = 1;
            }
            else
            {
                USART_rx_buffer[rx_wr_index] = LL_USART_ReceiveData8(DEBUG_USART);
                rx_wr_index++;
            }
        }        
    }        
    if (LL_USART_IsActiveFlag_PE(DEBUG_USART))// Parity Error
    {
        LL_USART_ClearFlag_PE(DEBUG_USART);
    }  
    if (LL_USART_IsActiveFlag_FE(DEBUG_USART))// Framing Error
    {
        LL_USART_ClearFlag_FE(DEBUG_USART);
    }  
    if (LL_USART_IsActiveFlag_ORE(DEBUG_USART))// OverRun Error
    {
        LL_USART_ClearFlag_ORE(DEBUG_USART);
    }  
    if (LL_USART_IsActiveFlag_NE(DEBUG_USART))// Noise error
    {
        LL_USART_ClearFlag_NE(DEBUG_USART);
    }
}
#endif


const char HEX_TABLE[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void BSP_UART_TxChar(char ch)
{
    LL_USART_TransmitData8(DEBUG_USART, ch);
    while (!LL_USART_IsActiveFlag_TC(DEBUG_USART));
    LL_USART_ClearFlag_TC(DEBUG_USART);
}

void BSP_UART_TxHex8(uint8_t hex)
{
    BSP_UART_TxChar(HEX_TABLE[(hex >> 4) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[hex & 0x0F]);
}

void BSP_UART_TxHex16(uint16_t hex)
{
    BSP_UART_TxChar(HEX_TABLE[(hex >> 12) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 8) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 4) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[hex & 0xF]);
}

void BSP_UART_TxHex32(uint32_t hex)
{
    BSP_UART_TxChar(HEX_TABLE[(hex >> 28) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 24) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 20) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 16) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 12) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 8) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 4) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[hex & 0xF]);
}

#define PRINTF_BUF_LENGTH 64
void uprintf(const char* format,...)
{
    char buf[PRINTF_BUF_LENGTH];
    va_list arg;
    va_start(arg,format);
    vsprintf(buf,format,arg);
    va_end(arg);
    #ifdef USART_ENABLEIT
    USART_bufsend(buf,PRINTF_BUF_LENGTH);
    #else
    BSP_UART_TxString(buf);
    #endif
}
