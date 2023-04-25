/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v30x_it.h"

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @function    NMI_Handler
 *
 * @brief       This function handles NMI exception.
 *
 * @return      none
 */
void NMI_Handler(void)
{
}

/*********************************************************************
 * @funciton    HardFault_Handler
 *
 * @brief       This function handles Hard Fault exception.
 *
 * @return      none
 */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/*********************************************************************
 * @funciton    EXTI4_IRQHandler
 *
 * @brief       This function handles EXTI4 exception.
 *
 * @return      none
 */
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        EXTI_ClearFlag(EXTI_Line4);
        GPIO_WriteBit(GPIOE, GPIO_Pin_11, !GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_11));
    }
}

/*********************************************************************
 * @funciton    EXTI5_IRQHandler
 *
 * @brief       This function handles EXTI5 exception.
 *
 * @return      none
 */
void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        EXTI_ClearFlag(EXTI_Line5);
        GPIO_WriteBit(GPIOE, GPIO_Pin_12, !GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_12));
    }
}
