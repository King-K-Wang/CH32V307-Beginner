/********************************** (C) COPYRIGHT *******************************
* File Name          : AP3216C.h
* Author             : K.Wang
* Version            : V1.2.0
* Date               : 2023/06/06
* Description        : Header of AP3216 sensor.
*                       - uint8_t AP3216C_Init(void)
*                       - void AP3216C_ReadData(uint16_t* ir, uint16_t* ps, uint16_t* als)
*******************************************************************************/

#ifndef __AP3216C_H
#define __AP3216C_H

#include "debug.h"

#define AP3216C_ADDR    0X1E	//AP3216C IIC address

uint8_t AP3216C_Init(void);
void AP3216C_ReadData(uint16_t* ir, uint16_t* ps, uint16_t* als);
#endif
