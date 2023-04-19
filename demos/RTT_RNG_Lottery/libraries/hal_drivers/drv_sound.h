/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-09-09     WCH        the first version
 */
#ifndef DRV_SOUND_H__
#define DRV_SOUND_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define AUDIO_FREQUENCY_192K ((rt_uint32_t) 192000)
#define AUDIO_FREQUENCY_096K ((rt_uint32_t) 96000)
#define AUDIO_FREQUENCY_048K ((rt_uint32_t) 48000)
#define AUDIO_FREQUENCY_044K ((rt_uint32_t) 44100)
#define AUDIO_FREQUENCY_032K ((rt_uint32_t) 32000)
#define AUDIO_FREQUENCY_022K ((rt_uint32_t) 22050)
#define AUDIO_FREQUENCY_016K ((rt_uint32_t) 16000)
#define AUDIO_FREQUENCY_011K ((rt_uint32_t) 11025)
#define AUDIO_FREQUENCY_008K ((rt_uint32_t)  8000)
#define AUDIO_FREQUENCY_DFT  ((rt_uint32_t)  2   )



struct drv_i2s
{
    SPI_TypeDef             *I2S_Handler;
    I2S_InitTypeDef         I2S_Init;
    DMA_Channel_TypeDef     *I2S_DMAHandler;
    DMA_InitTypeDef         I2S_DMAInit;
};








#endif
