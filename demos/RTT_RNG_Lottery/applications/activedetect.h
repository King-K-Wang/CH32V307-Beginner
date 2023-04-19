#ifndef __ACTIVEDETECT_H

#define __ACTIVEDETECT_H

#include "debug.h"
#include <Get_Data.h>


#define max_vc_con       3
#define frame_t          20 // 分帧时间长度ms
#define frame_mov_t      10 // 移帧时间长度ms
#define frame_len        (frame_t*ADC_FS/1000)     // 单帧长度
#define frame_mov_len    (frame_mov_t*ADC_FS/1000) // 移帧长度

typedef struct
{
    uint32_t mid_val;
    uint16_t n_thl;
    uint16_t z_thl;
    uint32_t s_thl;
}rt_atap_arg;

typedef struct
{
    uint16_t *start;    // 有效起始地址
    uint16_t *end;      // 有效结束地质
}rt_active_voice;


void environment_noise( uint16_t *noise, uint16_t n_len, rt_atap_arg *atap );
void active_detect( uint16_t *vc, uint16_t buf_len, rt_active_voice *valid_voice, rt_atap_arg *atap_arg);




#endif
