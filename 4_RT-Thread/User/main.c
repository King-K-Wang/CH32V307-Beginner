/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2020/04/30
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"

/* Global typedef */

/* Global define */
#define THREAD_PRIORITY         16
#define THREAD_STACK_SIZE       256
#define THREAD_TIMESLICE        10
/* Global Variable */

/*********************************************************************
 * @fn          LED_BLINK_INIT
 *
 * @brief       LED directly calls the underlying driver
 *
 * @return      none
 */
void LED_BLINK_INIT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void led1_thread_entry(void *parameter)
{
    while(1)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_11);
        rt_thread_delay(1000);
        GPIO_ResetBits(GPIOE, GPIO_Pin_11);
        rt_thread_delay(2000);
    }
}

void led2_thread_entry(void *parameter)
{
    while(1)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_12);
        rt_thread_delay(3000);
        GPIO_ResetBits(GPIOE, GPIO_Pin_12);
        rt_thread_delay(5000);
    }
}


/*********************************************************************
 * @function    main
 *
 * @brief       Main is just one of the threads, in addition to tshell, idle.
 *
 * @return      none
 */

int main(void)
{
    rt_kprintf("\r\n MCU: CH32V307\r\n");
    SystemCoreClockUpdate();
	rt_kprintf(" SysClk: %dHz\r\n",SystemCoreClock);
	rt_kprintf(" ChipID: %08x\r\n",DBGMCU_GetCHIPID());
    rt_kprintf(" www.wch.cn\r\n");
	LED_BLINK_INIT();

	rt_thread_t tid1 = RT_NULL;
	tid1 = rt_thread_create("led1", led1_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	rt_thread_t tid2 = RT_NULL;
	tid2 = rt_thread_create("led2", led2_thread_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

	if(tid1 != RT_NULL)
	    rt_thread_startup(tid1);
	if(tid2 != RT_NULL)
	    rt_thread_startup(tid2);

	while(1)
	{
	    rt_thread_delay(1000);
	}
}
