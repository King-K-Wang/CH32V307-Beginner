#ifndef _GET_DATA_H
#define _GET_DATA_H
#include "debug.h"


#define ADC_FS       8000                            //等间隔采样评论
#define FS_LEN_T     2000                            //采样时长ms
#define VDBUF_LEN    ((ADC_FS/1000)*FS_LEN_T)        //缓存数据长度
#define Noise_len_t  300                             //噪声采样长度ms
#define Noise_len    ((ADC_FS/1000)*Noise_len_t)      //噪声采样点数
#define start_delay  410



void DMA_Rx_Init( DMA_Channel_TypeDef* DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize );

#endif

