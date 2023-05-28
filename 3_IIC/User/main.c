/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 USART Print debugging routine:
 USART1_Tx(PA9).
 This example demonstrates using USART1(PA9) as a print debug port output.

*/

#include "debug.h"
#include "IIC.h"
#include "AP3216C.h"


/* Global typedef */

/* Global define */

/* Global Variable */


/*********************************************************************
 * @function    main
 *
 * @brief       Main program.
 *
 * @return      none
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();
	Delay_Init();
	USART_Printf_Init(115200);	
	printf("SystemClk:%d\r\n",SystemCoreClock);
	printf( "ChipID:%08x\r\n", DBGMCU_GetCHIPID() );
	printf("This is IIC sensor example\r\n");

	/* Init IIC2 with 200000 baud, master address 0x02 */
	IIC2_Init(20000, 0x02);

	while(AP3216C_Init())
	{
	    printf("AP3216C init fail\r\n");
	}
	printf("AP3216C init succeed\r\n");

	uint16_t Infrared = 0;
	uint16_t Distance = 0;
	uint16_t Light = 0;
	while(1)
    {
	    AP3216C_ReadData(&Infrared, &Distance, &Light);
	    printf("%d, %d, %d\n", Infrared, Distance, Light);
	    Delay_Ms(5000);
	}
}

