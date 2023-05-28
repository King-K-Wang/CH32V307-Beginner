/********************************** (C) COPYRIGHT *******************************
* File Name          : IIC.c
* Author             : WCH/Verimake
*                      K.Wang
* Version            : V2.0.0
* Date               : 2021/06/06
* Description        : IIC function.
*                       - void IIC2_Init(uint32_t bound ,uint16_t address)
*                       - uint8_t IIC_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
*                       - uint8_t IIC_ReadByte(uint8_t addr, uint8_t reg)
*                       ¡ýNot verified
*                       - uint8_t IIC_WriteLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
*                       - uint8_t IIC_ReadLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
*******************************************************************************/
#include "IIC.h"
#include "debug.h"

/*******************************************************************************
* @function     : IIC2_Init
* @brief        : Initializes the IIC2 peripheral.
* @parameter    : None
* @return       : None
*******************************************************************************/
void IIC2_Init(uint32_t bound ,uint16_t address)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitTSturcture;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE );
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure );

    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C2, &I2C_InitTSturcture);

    I2C_Cmd(I2C2, ENABLE);

    I2C_AcknowledgeConfig(I2C2, ENABLE);
}

/*******************************************************************************
* @function     : IIC_WaitEvent
* @brief        : wait IIC event,with timeout (65535 times).
* @parameter    : None
* @return       : 0 - success
*                 2 - timeout
*******************************************************************************/
uint8_t IIC_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
{
    uint16_t counter = 0xffff;
    while(!I2C_CheckEvent(I2Cx, I2C_EVENT))
    {
        counter--;
        if(counter == 0)
            return 2;
    }
    return 0;
}

/*******************************************************************************
 * @function    : IIC_ReadByte
 * @brief       : Read one byte with IIC
 * @parameter   : Register address
 * @return      : Data read
*******************************************************************************/
uint8_t IIC2_ReadByte(uint8_t addr, uint8_t reg)
{
    uint8_t res;

    I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_GenerateSTART(I2C2, ENABLE);

    /* Send device address and write command */
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C2, (addr << 1) | 0x00, I2C_Direction_Transmitter);

    /* Wait for response */
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 0;

    /* Write register address */
    I2C_SendData(I2C2,reg);

    I2C_GenerateSTART(I2C2, ENABLE );
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send device address and read command */
    I2C_Send7bitAddress(I2C2, ((addr << 1) | 0x01), I2C_Direction_Receiver);

    /* Wait for response */
    while(!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    I2C_AcknowledgeConfig(I2C2, DISABLE);

    while( I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE) ==  RESET);
    /* Read data, send nACK */
    res = I2C_ReceiveData(I2C2);

    /* Generate stop */
    I2C_GenerateSTOP(I2C2, ENABLE);
    return res;
}

/*******************************************************************************
 * @function    : IIC_WriteByte
 * @brief       : Read one byte with IIC
 * @parameter   : Register address
 * @return      : 0 - success
 *                2 - timeout
*******************************************************************************/
uint8_t IIC2_WriteByte(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t res;

    I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_GenerateSTART(I2C2, ENABLE);

    /* Send device address and write command */
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
            res = 2;
    I2C_Send7bitAddress(I2C2, (addr << 1) | 0x00, I2C_Direction_Transmitter);

    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) == RESET);

    /* Write register address */
    I2C_SendData(I2C2, reg);

    /* Wait for response */
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        res = 2;

    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) ==  RESET);
        I2C_SendData(I2C2, data);

    /* Generate stop */
    I2C_GenerateSTOP(I2C2, ENABLE);
    return res;
}

/*******************************************************************************
* @function     : IIC_WriteLen
* @brief        : IIC continuously write.
* @parameter    : Device address
* @parameter    : Register address
* @parameter    : Length of writing
* @parameter    : Buffer
* @return       : 0 - success
*                 2 - timeout
*******************************************************************************/
uint8_t IIC2_WriteLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    uint8_t i = 0;

    I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_GenerateSTART(I2C2, ENABLE);

    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        return 2;

    /* Send device address and write command */
    I2C_Send7bitAddress(I2C2, (addr<<1) | 0x00, I2C_Direction_Transmitter);

    /* Wait for response */
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 2;

    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) == RESET);

    /* Write register address */
    I2C_SendData(I2C2, reg);

    /* Send data */
    while(i < len)
    {
        if(I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) != RESET)
        {
            I2C_SendData(I2C2, buf[i]);
            i++;
        }
    }
    while(I2C_GetFlagStatus(I2C2, I2C_FLAG_TXE) == RESET);

    /* Generate stop */
    I2C_GenerateSTOP(I2C2, ENABLE);
    return 0;
}

/*******************************************************************************
* @function     : IIC_ReadLen
* @brief        : IIC continuously read.
* @parameter    : Device address
* @parameter    : Register address
* @parameter    : Length of reading
* @parameter    : Buffer
* @return       : 0 - success
*                 2 - timeout
*******************************************************************************/
uint8_t IIC2_ReadLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    uint8_t i = 0;

    I2C_AcknowledgeConfig(I2C2, ENABLE);
    I2C_GenerateSTART(I2C2, ENABLE);

    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        return 2;
    /* Send device address and write command */
    I2C_Send7bitAddress(I2C2, (addr << 1) | 0X00, I2C_Direction_Transmitter);

    /* Wait for response */
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 2;

    /* Write register address */
    I2C_SendData(I2C2, reg);

    I2C_GenerateSTART(I2C2, ENABLE);
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
        return 2;

    /* Send device address and read command */
    I2C_Send7bitAddress(I2C2, ((addr << 1) | 0x01), I2C_Direction_Receiver);
    if(IIC_WaitEvent(I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        return 2;

    while(i < len)
    {
        if(I2C_GetFlagStatus(I2C2, I2C_FLAG_RXNE) != RESET)
        {
            if(i == (len - 1))
            {
                I2C_AcknowledgeConfig(I2C2, DISABLE);
                /* Read data, send nACK */
                buf[i] = I2C_ReceiveData(I2C2);
            }
            else
            {
                /* Read data, send ACK */
                buf[i] = I2C_ReceiveData(I2C2);
            }
            i++;
        }
    }

    /* Generate stop */
    I2C_GenerateSTOP(I2C2, ENABLE);
    return 0;
}
