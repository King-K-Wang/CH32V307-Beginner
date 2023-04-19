/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2021-09-09     WCH        the first version
 */
#include <drv_sound.h>
#include "drivers/audio.h"
#include "drv_es8388.h"

#define DBG_TAG         "drv.sound"
#define DBG_LVL          DBG_INFO
#include <rtdbg.h>

#define CODEC_I2C_NAME   ("i2c2")

#define TX_DMA_FIFO_SIZE (2048)

struct drv_i2s _i2s_a = {0};
struct ch32_audio
{
    struct rt_audio_device     audio;
    struct rt_audio_configure  replay_config;
    int                        replay_volume;
    rt_uint8_t                 *tx_fifo;
};

struct ch32_audio _ch32_audio_play = {0};

void SAIA_samplerate_set(rt_uint32_t freq)
{
    I2S_Cmd(_i2s_a.I2S_Handler, DISABLE);
    _i2s_a.I2S_Init.I2S_AudioFreq = freq;
    I2S_Init(_i2s_a.I2S_Handler, &_i2s_a.I2S_Init);
    I2S_Cmd(_i2s_a.I2S_Handler, ENABLE);
}

void SAIA_channels_set(rt_uint16_t channels)
{
    (void)channels;
}

void SAIA_samplebits_set(rt_uint16_t samplebits)
{
    I2S_Cmd(_i2s_a.I2S_Handler, DISABLE);
    switch(samplebits)
    {
    case 16:
        _i2s_a.I2S_Init.I2S_DataFormat = I2S_DataFormat_16b;
        break;
    case 24:
        _i2s_a.I2S_Init.I2S_DataFormat = I2S_DataFormat_24b;
        break;
    case 32:
        _i2s_a.I2S_Init.I2S_DataFormat = I2S_DataFormat_32b;
        break;
    default :
        _i2s_a.I2S_Init.I2S_DataFormat = I2S_DataFormat_16b;
        break;
    }
    I2S_Init(_i2s_a.I2S_Handler, &_i2s_a.I2S_Init);
    I2S_Cmd(_i2s_a.I2S_Handler, ENABLE);
}

void SAIA_config_set(struct rt_audio_configure  config)
{
     SAIA_channels_set(config.channels);
     SAIA_samplebits_set(config.samplebits);
     SAIA_samplerate_set(config.samplerate);
}

/* initial I2S2 */
rt_err_t i2s_config_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    _i2s_a.I2S_Handler             = SPI2;
    _i2s_a.I2S_Init.I2S_Mode       = I2S_Mode_MasterTx;
    _i2s_a.I2S_Init.I2S_Standard   = I2S_Standard_Phillips;
    _i2s_a.I2S_Init.I2S_DataFormat = I2S_DataFormat_16b;
    _i2s_a.I2S_Init.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
    _i2s_a.I2S_Init.I2S_AudioFreq  = I2S_AudioFreq_44k;
    _i2s_a.I2S_Init.I2S_CPOL       = I2S_CPOL_High;
    I2S_Init(_i2s_a.I2S_Handler, &_i2s_a.I2S_Init);

    SPI_I2S_DMACmd(_i2s_a.I2S_Handler, SPI_I2S_DMAReq_Tx, ENABLE);
    I2S_Cmd(_i2s_a.I2S_Handler, ENABLE);

    return RT_EOK;
}

rt_err_t i2s_tx_dma(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    _i2s_a.I2S_DMAHandler = DMA1_Channel5;
    DMA_DeInit(_i2s_a.I2S_DMAHandler);
    _i2s_a.I2S_DMAInit.DMA_PeripheralBaseAddr = (rt_uint32_t)&SPI2->DATAR;
    _i2s_a.I2S_DMAInit.DMA_MemoryBaseAddr     = (rt_uint32_t)_ch32_audio_play.tx_fifo;
    _i2s_a.I2S_DMAInit.DMA_DIR                = DMA_DIR_PeripheralDST;
    _i2s_a.I2S_DMAInit.DMA_BufferSize         = TX_DMA_FIFO_SIZE / 2 ;
    _i2s_a.I2S_DMAInit.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    _i2s_a.I2S_DMAInit.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    _i2s_a.I2S_DMAInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    _i2s_a.I2S_DMAInit.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    _i2s_a.I2S_DMAInit.DMA_Mode               = DMA_Mode_Circular;
    _i2s_a.I2S_DMAInit.DMA_Priority           = DMA_Priority_Medium ;
    _i2s_a.I2S_DMAInit.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(_i2s_a.I2S_DMAHandler, &_i2s_a.I2S_DMAInit);
    DMA_ClearITPendingBit(DMA1_IT_HT5|DMA1_IT_TC5|DMA1_IT_GL5|DMA1_IT_TE5);
    DMA_ITConfig(_i2s_a.I2S_DMAHandler, DMA1_IT_TC5|DMA1_IT_HT5, ENABLE);

    NVIC_SetPriority(DMA1_Channel5_IRQn,0xE0);     /* 设置优先级 */
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);
    DMA_Cmd(_i2s_a.I2S_DMAHandler, DISABLE);

    return RT_EOK;
}



void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void)
{
    GET_INT_SP();
    rt_interrupt_enter();

    if(DMA_GetITStatus(DMA1_IT_TC5)) /* 传输完成 */
    {
        rt_kprintf("DMA\r\n");
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        rt_audio_tx_complete(&_ch32_audio_play.audio);
    }
    if(DMA_GetITStatus(DMA1_IT_HT5)) /* 传输过半 */
    {
        rt_kprintf("DMA half\r\n");
        DMA_ClearITPendingBit(DMA1_IT_HT5);
        rt_audio_tx_complete(&_ch32_audio_play.audio);
    }

    rt_interrupt_leave();
    FREE_INT_SP();
}


//void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
//{
//    rt_audio_tx_complete(&_stm32_audio_play.audio);
//}
//
//void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
//{
//    rt_audio_tx_complete(&_stm32_audio_play.audio);
//}


rt_err_t sai_a_init()
{
    i2s_config_init();
    /* set sai_a DMA */
    i2s_tx_dma();

    return RT_EOK;
}


static rt_err_t ch32_player_getcaps(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    rt_err_t result = RT_EOK;
    struct ch32_audio *ch_audio = (struct ch32_audio *)audio->parent.user_data;

    LOG_D("%s:main_type: %d, sub_type: %d", __FUNCTION__, caps->main_type, caps->sub_type);

    switch (caps->main_type)
    {
    case AUDIO_TYPE_QUERY: /* query the types of hw_codec device */
    {
        switch (caps->sub_type)
        {
        case AUDIO_TYPE_QUERY:
            caps->udata.mask = AUDIO_TYPE_OUTPUT | AUDIO_TYPE_MIXER;
            break;

        default:
            result = -RT_ERROR;
            break;
        }

        break;
    }

    case AUDIO_TYPE_OUTPUT: /* Provide capabilities of OUTPUT unit */
    {
        switch (caps->sub_type)
        {
        case AUDIO_DSP_PARAM:
            caps->udata.config.channels     = ch_audio->replay_config.channels;
            caps->udata.config.samplebits   = ch_audio->replay_config.samplebits;
            caps->udata.config.samplerate   = ch_audio->replay_config.samplerate;
            break;

        case AUDIO_DSP_SAMPLERATE:
            caps->udata.config.samplerate   = ch_audio->replay_config.samplerate;
            break;

        case AUDIO_DSP_CHANNELS:
            caps->udata.config.channels     = ch_audio->replay_config.channels;
            break;

        case AUDIO_DSP_SAMPLEBITS:
            caps->udata.config.samplebits   = ch_audio->replay_config.samplebits;
            break;

        default:
            result = -RT_ERROR;
            break;
        }

        break;
    }

    case AUDIO_TYPE_MIXER: /* report the Mixer Units */
    {
        switch (caps->sub_type)
        {
        case AUDIO_MIXER_QUERY:
            caps->udata.mask = AUDIO_MIXER_VOLUME /*| AUDIO_MIXER_LINE*/;
            break;

        case AUDIO_MIXER_VOLUME:
            caps->udata.value = es8388_volume_get();
            break;

//        case AUDIO_MIXER_LINE:
//            break;

        default:
            result = -RT_ERROR;
            break;
        }

        break;
    }

    default:
        result = -RT_ERROR;
        break;
    }

    return result;
}



static rt_err_t  ch32_player_configure(struct rt_audio_device *audio, struct rt_audio_caps *caps)
{
    rt_err_t result = RT_EOK;
    struct ch32_audio *ch_audio = (struct ch32_audio *)audio->parent.user_data;

    LOG_D("%s:main_type: %d, sub_type: %d", __FUNCTION__, caps->main_type, caps->sub_type);

    switch (caps->main_type)
        {
        case AUDIO_TYPE_MIXER:
        {
            switch (caps->sub_type)
            {
            case AUDIO_MIXER_VOLUME:
            {
                rt_uint8_t volume = caps->udata.value;

                es8388_volume_set(volume);
                ch_audio->replay_volume = volume;
                LOG_D("set volume %d", volume);
                break;
            }

            default:
                result = -RT_ERROR;
                break;
            }

            break;
        }

        case AUDIO_TYPE_OUTPUT:
        {
            switch (caps->sub_type)
            {
            case AUDIO_DSP_PARAM:
            {
                /* set samplerate */
                SAIA_samplerate_set(caps->udata.config.samplerate);
                /* set channels */
                SAIA_channels_set(caps->udata.config.channels);

                /* save configs */
                ch_audio->replay_config.samplerate = caps->udata.config.samplerate;
                ch_audio->replay_config.channels   = caps->udata.config.channels;
                ch_audio->replay_config.samplebits = caps->udata.config.samplebits;
                LOG_D("set samplerate %d", ch_audio->replay_config.samplerate);
                break;
            }

            case AUDIO_DSP_SAMPLERATE:
            {
                SAIA_samplerate_set(caps->udata.config.samplerate);
                ch_audio->replay_config.samplerate = caps->udata.config.samplerate;
                LOG_D("set samplerate %d", ch_audio->replay_config.samplerate);
                break;
            }

            case AUDIO_DSP_CHANNELS:
            {
                SAIA_channels_set(caps->udata.config.channels);
                ch_audio->replay_config.channels   = caps->udata.config.channels;
                LOG_D("set channels %d", ch_audio->replay_config.channels);
                break;
            }

            case AUDIO_DSP_SAMPLEBITS:
            {
                /* not support */
                ch_audio->replay_config.samplebits = caps->udata.config.samplebits;
                break;
            }

            default:
                result = -RT_ERROR;
                break;
            }

            break;
        }

        default:
            break;
        }

        return result;
}


static rt_err_t ch32_player_init(struct rt_audio_device *audio)
{
    struct ch32_audio *ch_audio = (struct ch32_audio *)audio->parent.user_data;

    es8388_init(CODEC_I2C_NAME, 8); //8---->PA8

    sai_a_init();

    /* set default params */
    SAIA_samplerate_set(ch_audio->replay_config.samplerate);
    SAIA_channels_set(ch_audio->replay_config.channels);

    LOG_I("ch32_player_init\r\n");
    return RT_EOK;
}


static rt_err_t ch32_player_start(struct rt_audio_device *audio, int stream)
{
//    struct ch32_audio *ch_audio = (struct ch32_audio *)audio->parent.user_data;
    if (stream == AUDIO_STREAM_REPLAY)
    {
        LOG_D("open sound device");
        es8388_start(ES_MODE_DAC);
        DMA_Cmd(_i2s_a.I2S_DMAHandler, ENABLE);
//        HAL_SAI_Transmit_DMA(&SAI1A_Handler, ch_audio->tx_fifo, TX_DMA_FIFO_SIZE / 2);
    }
    return RT_EOK;
}


static rt_err_t ch32_player_stop(struct rt_audio_device *audio, int stream)
{
    if (stream == AUDIO_STREAM_REPLAY)
    {
        DMA_Cmd(_i2s_a.I2S_DMAHandler, DISABLE);
        es8388_stop(ES_MODE_DAC);
        LOG_D("close sound device");
    }

    return RT_EOK;
}


static void ch32_player_buffer_info(struct rt_audio_device *audio, struct rt_audio_buf_info *info)
{
    /**
     *               TX_FIFO
     * +----------------+----------------+
     * |     block1     |     block2     |
     * +----------------+----------------+
     *  \  block_size  /
     */
    info->buffer = _ch32_audio_play.tx_fifo;
    info->total_size = TX_DMA_FIFO_SIZE;
    info->block_size = TX_DMA_FIFO_SIZE / 2;
    info->block_count = 2;
}
static struct rt_audio_ops _p_audio_ops =
{
    .getcaps     = ch32_player_getcaps,
    .configure   = ch32_player_configure,
    .init        = ch32_player_init,
    .start       = ch32_player_start,
    .stop        = ch32_player_stop,
    .transmit    = RT_NULL,
    .buffer_info = ch32_player_buffer_info,
};

int rt_hw_sound_init(void)
{
    rt_uint8_t *tx_fifo;

    /* player */
    tx_fifo = rt_malloc(TX_DMA_FIFO_SIZE);
    if (tx_fifo == RT_NULL)
    {
        return -RT_ENOMEM;
    }
    rt_memset(tx_fifo, 0, TX_DMA_FIFO_SIZE);
    _ch32_audio_play.tx_fifo = tx_fifo;

    /* init default configuration */
    {
        _ch32_audio_play.replay_config.samplerate = 44100;
        _ch32_audio_play.replay_config.channels   = 2;
        _ch32_audio_play.replay_config.samplebits = 16;
        _ch32_audio_play.replay_volume            = 55;
    }




    /* register sound device */
    _ch32_audio_play.audio.ops = &_p_audio_ops;
    rt_audio_register(&_ch32_audio_play.audio, "sound0", RT_DEVICE_FLAG_WRONLY, &_ch32_audio_play);
    return RT_EOK;
}

INIT_DEVICE_EXPORT(rt_hw_sound_init);




