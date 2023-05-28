/********************************** (C) COPYRIGHT *******************************
* File Name          : AP3216C.c
* Author             : K.Wang
* Version            : V1.2.0
* Date               : 2023/06/06
* Description        : AP3216C Drivers.
*                       - uint8_t AP3216C_Init(void)
*                       - void AP3216C_ReadData(uint16_t* ir, uint16_t* ps, uint16_t* als)
*******************************************************************************/

#include "AP3216C.h"
#include "debug.h"
#include "IIC.h"

/**
 * @brief	     : Initialize AP3216C
 * @param        : None
 * @return       : 0 - success
 *                 1 - fail
 */
uint8_t AP3216C_Init(void)
{
    uint8_t temp = 0;
    /* Reset AP3216C */
    IIC2_WriteByte(AP3216C_ADDR, 0x00, 0X04);
    Delay_Ms(50);

    /* Open ALS PS IR*/
    IIC2_WriteByte(AP3216C_ADDR, 0x00, 0X03);
    /* Read back */
    temp = IIC2_ReadByte(AP3216C_ADDR, 0X00);

    if(temp == 0X03)
        return 0;
    else
        return 1;
}

/**
 * @brief        : Read original data of AP3216C, including ALS, PS, IR
 *			       If all opened, span at least 112.5ms
 * @param        : ir - Infrared data
 * @param        : ps - Detection distance data
 * @param        : als - Ambient light sensor data
 * @return       : None
 */
void AP3216C_ReadData(uint16_t* ir, uint16_t* ps, uint16_t* als)
{
    uint8_t buf[6];

    for(uint8_t i = 0; i < 6; i++)
    {
        /* Read all the data registers */
        buf[i] = IIC2_ReadByte(AP3216C_ADDR, 0X0A + i);
    }

    /* IR_OF == 1, either data invalid */
    if(buf[0] & 0X80)
        *ir = 0;
    else
        /* Read IR data */
        *ir = ((uint16_t)buf[1] << 2) | (buf[0] & 0X03);

    /*Read ALS data*/
    *als = ((u16)buf[3] << 8) | buf[2];

    /* IR_OF == 1, either data invalid */
    if(buf[4] & 0x40)
        *ps = 0;
    else
        /* Read PS data */
        *ps = ((u16)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F);
}
