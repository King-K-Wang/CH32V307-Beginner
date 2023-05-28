/********************************** (C) COPYRIGHT *******************************
* File Name          : IIC.h
* Author             : WCH/Verimake
*                      K.Wang
* Version            : V2.0.0
* Date               : 2023/06/06
* Description        : Header of IIC function.
*                       - void IIC2_Init(uint32_t bound ,uint16_t address)
*                       - uint8_t IIC_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)
*                       - uint8_t IIC_ReadByte(uint8_t addr, uint8_t reg)
*                       ¡ýNot verified
*                       - uint8_t IIC_WriteLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
*                       - uint8_t IIC_ReadLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
*******************************************************************************/
#ifndef __IIC_H
#define __IIC_H

#include "debug.h"

extern void IIC2_Init(uint32_t bound ,uint16_t address);
extern uint8_t IIC_WaitEvent(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT);

uint8_t IIC2_ReadByte(uint8_t addr, uint8_t reg);
uint8_t IIC2_WriteByte(uint8_t addr, uint8_t reg, uint8_t data);

uint8_t IIC2_WriteLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
uint8_t IIC2_ReadLen(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
#endif
