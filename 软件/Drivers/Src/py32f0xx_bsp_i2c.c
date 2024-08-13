#include "py32f0xx_bsp_i2c.h"

void I2C_Unlock(void);
void BSP_I2C_Config(void)
{
    I2C_Unlock();
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    //LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    //SCL
    GPIO_InitStruct.Pin = SCL1_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
    LL_GPIO_Init(SCL1_PORT, &GPIO_InitStruct);
    //SDA
    GPIO_InitStruct.Pin = SDA1_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
    LL_GPIO_Init(SDA1_PORT, &GPIO_InitStruct);

    
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);

    LL_I2C_EnableReset(I2C1);
    LL_I2C_DisableReset(I2C1);

    LL_I2C_InitTypeDef I2C_InitStruct;
    /* 
    * Clock speed:
    * - standard = 100khz, if PLL is on, set system clock <= 16MHz, or i2c cannot work normally
    * - fast     = 400khz
    */
    I2C_InitStruct.ClockSpeed      = LL_I2C_MAX_SPEED_FAST;
    I2C_InitStruct.DutyCycle       = LL_I2C_DUTYCYCLE_16_9;
    I2C_InitStruct.OwnAddress1     = I2C_ADDRESS;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_NACK;
    LL_I2C_Init(I2C1, &I2C_InitStruct);

    /* Enale clock stretch (reset default: on) */
    // LL_I2C_EnableClockStretching(I2C1);
  
    /* Enable general call (reset default: off) */
    // LL_I2C_EnableGeneralCall(I2C1);
    #ifdef I2C_USE_IT
        NVIC_SetPriority(I2C1_IRQn, 0);
        NVIC_EnableIRQ(I2C1_IRQn);
        LL_I2C_EnableIT_ERR(I2C1);
    #endif
    LL_I2C_DisableIT_BUF(I2C1);
    LL_I2C_DisableIT_EVT(I2C1);   
}

ErrorStatus APP_I2C_TestAddress(uint8_t dev_addr)
{
    uint16_t timeout = 0xFFF;
    while(LL_I2C_IsActiveFlag_BUSY(I2C1));
    /* Disable Pos */
    LL_I2C_DisableBitPOS(I2C1);
    /* Generate Start */
    LL_I2C_GenerateStartCondition(I2C1);
    /* Wait until SB flag is set */
    while(LL_I2C_IsActiveFlag_SB(I2C1) != 1);
    /* Send slave address */
    LL_I2C_TransmitData8(I2C1, (dev_addr & (uint8_t)(~0x01)));
    /* Wait until ADDR flag is set */
    while(LL_I2C_IsActiveFlag_ADDR(I2C1) != 1)
    {
        timeout--;
        if (timeout == 0) break;
    }
    if (timeout == 0)
    {
        LL_I2C_GenerateStopCondition(I2C1);
        return ERROR;
    }
    else
    {
        LL_I2C_ClearFlag_ADDR(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        return SUCCESS;
    }
}

void APP_I2C_Transmit(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t len)
{
    while(LL_I2C_IsActiveFlag_BUSY(I2C1));
    LL_I2C_DisableBitPOS(I2C1);

    /* Start */
    LL_I2C_GenerateStartCondition(I2C1);
    while (LL_I2C_IsActiveFlag_SB(I2C1) != 1);
    /* Send slave address */
    LL_I2C_TransmitData8(I2C1, (devAddress & (uint8_t)(~0x01)));
    while (LL_I2C_IsActiveFlag_ADDR(I2C1) != 1);
    LL_I2C_ClearFlag_ADDR(I2C1);

    /* Send memory address */
    LL_I2C_TransmitData8(I2C1, memAddress);
    while (LL_I2C_IsActiveFlag_TXE(I2C1) != 1);

    /* Transfer data */
    while (len > 0)
    {
        while (LL_I2C_IsActiveFlag_TXE(I2C1) != 1);
        LL_I2C_TransmitData8(I2C1, *pData++);
        len--;

        if ((LL_I2C_IsActiveFlag_BTF(I2C1) == 1) && (len != 0U))
        {
            LL_I2C_TransmitData8(I2C1, *pData++);
            len--;
        }

        //while (LL_I2C_IsActiveFlag_BTF(I2C1) != 1);
    }

    /* Stop */
    LL_I2C_GenerateStopCondition(I2C1);
}

void APP_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    while(LL_I2C_IsActiveFlag_BUSY(I2C1));
    /* Disable Pos */
    LL_I2C_DisableBitPOS(I2C1);

    /***** Send device address + memory address *****/

    /* Enable Acknowledge */
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
    /* Generate Start */
    LL_I2C_GenerateStartCondition(I2C1);
    /* Wait until SB flag is set */
    while(LL_I2C_IsActiveFlag_SB(I2C1) != 1);

    /* Send slave address */
    LL_I2C_TransmitData8(I2C1, (dev_addr & (uint8_t)(~0x01)));
    /* Wait until ADDR flag is set */
    while(LL_I2C_IsActiveFlag_ADDR(I2C1) != 1);
    /* Clear ADDR flag */
    LL_I2C_ClearFlag_ADDR(I2C1);

    /* Wait until TXE flag is set */
    while(LL_I2C_IsActiveFlag_TXE(I2C1) != 1);
    /* Send memory address */
    LL_I2C_TransmitData8(I2C1, (uint8_t)(reg_addr & 0x00FF));
    while(LL_I2C_IsActiveFlag_BTF(I2C1) != 1);

    /***** Restart to read *****/

    /* Generate Start */
    LL_I2C_GenerateStartCondition(I2C1);
    /* Wait until SB flag is set */
    while(LL_I2C_IsActiveFlag_SB(I2C1) != 1);

    /* Send slave address */
    LL_I2C_TransmitData8(I2C1, (dev_addr | 0x1));
    /* Wait until ADDR flag is set */
    while(LL_I2C_IsActiveFlag_ADDR(I2C1) != 1);

    if (len == 0U)
    {
        LL_I2C_ClearFlag_ADDR(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
    }
    else if(len == 1U)
    {
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

        __disable_irq();
        LL_I2C_ClearFlag_ADDR(I2C1);
        LL_I2C_GenerateStopCondition(I2C1);
        __enable_irq();
    }
    else if(len == 2U)
    {
        LL_I2C_EnableBitPOS(I2C1);

        __disable_irq();
        LL_I2C_ClearFlag_ADDR(I2C1);
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
        __enable_irq();
    }
    else
    {
        LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
        LL_I2C_ClearFlag_ADDR(I2C1);
    }

    while (len > 0U)
    {
        if (len <= 3U)
        {
            if (len == 1U)
            {
                while(LL_I2C_IsActiveFlag_RXNE(I2C1) != 1);
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
            }
            else if (len == 2U)
            {
                while(LL_I2C_IsActiveFlag_BTF(I2C1) != 1);
            
                __disable_irq();
                LL_I2C_GenerateStopCondition(I2C1);
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
                __enable_irq();
            
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
            }
            else
            {
                while(LL_I2C_IsActiveFlag_BTF(I2C1) != 1);
            
                LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
            
                __disable_irq();
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
                while(LL_I2C_IsActiveFlag_BTF(I2C1) != 1);
                LL_I2C_GenerateStopCondition(I2C1);
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
                __enable_irq();
            
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
            }
        }
        else
        {
            while(LL_I2C_IsActiveFlag_RXNE(I2C1) != 1);
          
            *data++ = LL_I2C_ReceiveData8(I2C1);
            len--;
          
            if (LL_I2C_IsActiveFlag_BTF(I2C1) == 1)
            {
                *data++ = LL_I2C_ReceiveData8(I2C1);
                len--;
            }
        }
        
    }
}

//I2C系统状态与异步操作传递数据用全局变量
//在空闲时被第一个ASYNC函数装填值，I2C传输过程中无法被外部修改
volatile struct I2C_StatusTypedef I2C1_Status;

void I2C_Unlock(void)
{
    uint8_t i,delay;
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    //SCL
    LL_GPIO_ResetOutputPin(SCL1_PORT,SCL1_PIN);
    GPIO_InitStruct.Pin = SCL1_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
    LL_GPIO_Init(SCL1_PORT, &GPIO_InitStruct);
    //SDA
    LL_GPIO_SetOutputPin(SDA1_PORT,SDA1_PIN);
    GPIO_InitStruct.Pin = SDA1_PIN;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
    LL_GPIO_Init(SDA1_PORT, &GPIO_InitStruct);
    
    if(LL_GPIO_IsInputPinSet(SDA1_PORT,SDA1_PIN)==0)
    {
        for(i = 0;i<9;i++)
        {
            LL_GPIO_SetOutputPin(SCL1_PORT,SCL1_PIN);
            delay = 255;
            while(delay > 1) delay--;
            LL_GPIO_ResetOutputPin(SCL1_PORT,SCL1_PIN);
            delay = 255;
            while(delay > 1) delay--;
            if(LL_GPIO_IsInputPinSet(SDA1_PORT,SDA1_PIN)==0) break;
        }
    }
}

void I2C_Diagnosis(void)
{
    LL_I2C_EnableReset(I2C1);
    BSP_I2C_Config();
    I2C1_Status.i2c1_status = I2C_IDLE;
}

#ifdef I2C_WDT_THREAD
uint32_t i2cwdtovf;
THRD_DECLARE(thread_i2c_wdt)
{
    THRD_BEGIN;
    while(1)
    {
        if(I2C1_Status.i2c1_status != I2C_IDLE && i2cwdtovf > millis) I2C_Diagnosis();//只在I2C活动状态下生效
        if(LL_I2C_IsActiveFlag_BUSY(I2C1) && I2C1_Status.i2c1_status == I2C_IDLE) I2C_Diagnosis();
        THRD_YIELD;
    }
    THRD_END;
}
#endif

//向异步进程发布发送任务，成功时返回1
uint8_t ASYNC_I2C_Transmit(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t *flag)
{
    if(LL_I2C_IsActiveFlag_BUSY(I2C1) == 0 && I2C1_Status.i2c1_status == I2C_IDLE)
    {
        I2C1_Status.i2c1_status = I2C_TX_ADDR;
        I2C1_Status.async_dev_addr = dev_addr & (uint8_t)(~0x01);
        I2C1_Status.async_reg_addr = reg_addr;
        I2C1_Status.async_data = data;
        I2C1_Status.async_len = len;
        I2C1_Status.async_flag = flag;
        *I2C1_Status.async_flag = 0;
        #ifdef I2C_USE_IT
            LL_I2C_EnableIT_TX(I2C1);
        #endif
        #ifdef I2C_WDT_THREAD                                                   //设置超时
            i2cwdtovf = millis + I2C_TIMEOUT_TIME;
        #endif
        LL_I2C_DisableBitPOS(I2C1);
        LL_I2C_GenerateStartCondition(I2C1);
        return 1;
    }
    else return 0;
}
//向异步进程发布接收任务，成功时返回1
uint8_t ASYNC_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t *flag)
{
    if(LL_I2C_IsActiveFlag_BUSY(I2C1) == 0 && I2C1_Status.i2c1_status == I2C_IDLE)
    {
        I2C1_Status.i2c1_status = I2C_RX_POINTER_ADDR;
        I2C1_Status.async_dev_addr = dev_addr & (uint8_t)(~0x01);
        I2C1_Status.async_reg_addr = reg_addr;
        I2C1_Status.async_data = data;
        I2C1_Status.async_len = len;
        I2C1_Status.async_flag = flag;
        *I2C1_Status.async_flag = 0;      
        #ifdef I2C_USE_IT
            LL_I2C_EnableIT_TX(I2C1);
        #endif
        #ifdef I2C_WDT_THREAD                                                   //设置超时
            i2cwdtovf = millis + I2C_TIMEOUT_TIME;
        #endif  
        LL_I2C_DisableBitPOS(I2C1);
        LL_I2C_GenerateStartCondition(I2C1);
        return 1;
    }
    else return 0;
}


#ifdef I2C_USE_IT
void I2C1_IRQHandler()//该中断服务函数名称在startup_py32f030x6.h中定义
{
    #ifdef I2C_WDT_THREAD                                                       //喂狗
        i2cwdtovf = millis + I2C_TIMEOUT_TIME;
    #endif
    //正常传输
    if(LL_I2C_IsActiveFlag_SB(I2C1))//START发送完成(EV5)
    {
        switch(I2C1_Status.i2c1_status)
        {
            case I2C_TX_ADDR:
            case I2C_RX_POINTER_ADDR:
                LL_I2C_TransmitData8(I2C1,I2C1_Status.async_dev_addr);//发送从机地址，写入
                break;
            case I2C_RX:
                LL_I2C_TransmitData8(I2C1,(I2C1_Status.async_dev_addr | 0x01));//发送从机地址，读取
                break;
            default:
                break;
        }
    }
    if(LL_I2C_IsActiveFlag_ADDR(I2C1))//从机响应(EV6)
    {
        LL_I2C_ClearFlag_ADDR(I2C1);//清标志位
        switch(I2C1_Status.i2c1_status)
        {
            case I2C_TX_ADDR:
                I2C1_Status.i2c1_status = I2C_TX_ACKED;
                break;
            case I2C_RX_POINTER_ADDR:
                I2C1_Status.i2c1_status = I2C_RX_POINTER_ACKED;
                break;
            case I2C_RX://读取模式发送完从机地址，这时已经启动字节接收
                if (I2C1_Status.async_len == 0U) 
                {
                    LL_I2C_DisableIT_RX(I2C1);
                    LL_I2C_GenerateStopCondition(I2C1);
                    LL_I2C_DisableIT_RX(I2C1);
                    *I2C1_Status.async_flag = I2C_FLAG_DONE;
                    I2C1_Status.i2c1_status = I2C_IDLE;//0字节接收结束了
                }
                else if(I2C1_Status.async_len == 1U)
                {
                    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
                    LL_I2C_GenerateStopCondition(I2C1);
                }
                else if(I2C1_Status.async_len == 2U)
                {
                    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
                }
                else LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
                break;
            default:
                break;
        }
    }
    if(LL_I2C_IsActiveFlag_BTF(I2C1))//EV8_2
    {
        switch(I2C1_Status.i2c1_status)
        {
            case I2C_RX_POINTER_SENT://寄存器地址发送完成
                LL_I2C_DisableIT_TX(I2C1);
                LL_I2C_EnableIT_RX(I2C1);
                I2C1_Status.i2c1_status = I2C_RX;
                LL_I2C_GenerateStartCondition(I2C1);//Repeat Start
                break;
            case I2C_RX:
                if(I2C1_Status.async_len == 2U)// 2/3字节接收等到了BTF
                {
                    LL_I2C_GenerateStopCondition(I2C1);
                    *I2C1_Status.async_data++ = LL_I2C_ReceiveData8(I2C1);
                    I2C1_Status.async_len--;
                    *I2C1_Status.async_data++ = LL_I2C_ReceiveData8(I2C1);
                    I2C1_Status.async_len--;
                    LL_I2C_DisableIT_RX(I2C1);
                    *I2C1_Status.async_flag = I2C_FLAG_DONE;
                    I2C1_Status.i2c1_status = I2C_IDLE;// 2/3字节接收结束了
                }
                else if(I2C1_Status.async_len == 3U)//3字节接收等到了BTF
                {
                    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
                    *I2C1_Status.async_data++ = LL_I2C_ReceiveData8(I2C1);
                    I2C1_Status.async_len--;
                }
                break;
            default:
                break;
        }
    }
    if(LL_I2C_IsActiveFlag_RXNE(I2C1) && I2C1_Status.async_len > 0U && I2C1_Status.i2c1_status == I2C_RX)
    { //EV7，接收区非空。EV6后，其将自动产生(除了len==0的情况)
        switch(I2C1_Status.async_len)
        {
            case 1U:
                *I2C1_Status.async_data++ = LL_I2C_ReceiveData8(I2C1);//先收数据
                I2C1_Status.async_len--;
                LL_I2C_DisableIT_RX(I2C1);
                *I2C1_Status.async_flag = I2C_FLAG_DONE;
                I2C1_Status.i2c1_status = I2C_IDLE;//1字节接收结束了
                break;
            case 2U:
                LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);//不响应下一个数据
                LL_I2C_DisableIT_BUF(I2C1);//关RXNE中断
                break;
            case 3U:
                LL_I2C_DisableIT_BUF(I2C1);//关RXNE中断
                break;
            default:
                *I2C1_Status.async_data++ = LL_I2C_ReceiveData8(I2C1);//先收数据
                I2C1_Status.async_len--;
                if (LL_I2C_IsActiveFlag_BTF(I2C1) == 1)//如果有两个字节待接收
                {
                    *I2C1_Status.async_data++ = LL_I2C_ReceiveData8(I2C1);//再收一个
                    I2C1_Status.async_len--;
                }
                break;            
        }                            
    }
    if(LL_I2C_IsActiveFlag_TXE(I2C1))//发送区为空(EV8)
    {
        switch(I2C1_Status.i2c1_status)
        {
            case I2C_TX_ACKED://(EV8_1)
                LL_I2C_TransmitData8(I2C1,I2C1_Status.async_reg_addr);//发送从机寄存器地址                
                I2C1_Status.i2c1_status = I2C_TX_ING;
                break;
            case I2C_TX_ING:
                if(I2C1_Status.async_len > 0)
                {
                    LL_I2C_TransmitData8(I2C1, *I2C1_Status.async_data--);
                    I2C1_Status.async_len--;
                }
                else if(LL_I2C_IsActiveFlag_BTF(I2C1) == 0) LL_I2C_DisableIT_BUF(I2C1);//最后一位数据已装入，关TXE中断
                else//最后一位数据已发送(EV8_2)(实际是BTF引发的中断走到这里)
                {   
                    LL_I2C_DisableIT_EVT(I2C1);
                    LL_I2C_GenerateStopCondition(I2C1);
                    *I2C1_Status.async_flag = I2C_FLAG_DONE;
                    I2C1_Status.i2c1_status = I2C_IDLE;//发送结束了
                }
                break;
            case I2C_RX_POINTER_ACKED:
                LL_I2C_TransmitData8(I2C1,I2C1_Status.async_reg_addr);//发送从机寄存器地址
                I2C1_Status.i2c1_status = I2C_RX_POINTER_SENT;
                break;
            default:
                break;
        }
    } 
    
    //错误处理
    if(LL_I2C_IsActiveFlag_AF(I2C1))//无应答
    {
        LL_I2C_ClearFlag_AF(I2C1);
        //I2C_Diagnosis();
        LL_I2C_DisableIT_BUF(I2C1);
        LL_I2C_DisableIT_EVT(I2C1);   
        LL_I2C_GenerateStopCondition(I2C1);
        I2C1_Status.i2c1_status = I2C_IDLE;
        *I2C1_Status.async_flag = I2C_FLAG_NORESPONSE;
    }
    if(LL_I2C_IsActiveFlag_BERR(I2C1))//总线错误
    {
        LL_I2C_ClearFlag_BERR(I2C1);
        I2C_Diagnosis();
        LL_I2C_DisableIT_BUF(I2C1);
        LL_I2C_DisableIT_EVT(I2C1);   
        LL_I2C_GenerateStopCondition(I2C1);
        I2C1_Status.i2c1_status = I2C_IDLE;
        *I2C1_Status.async_flag = I2C_FLAG_BUSERROR;
    }
}
#endif
