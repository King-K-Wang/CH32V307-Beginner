#ifndef __CALCMFCC_H
#define __CALCMFCC_H

#include "debug.h"
#include <activedetect.h>

#define mfcc_num     12       //mfcc阶数
#define vv_max_t     1200     //ms
#define vv_frm_max   ((vv_max_t-frame_t)/(frame_t-frame_mov_t)+1)

/* fft用复数  */
typedef struct
{
    float real;
    float imag;
}rt_compx_v;

typedef struct __attribute__((packed))
{
    uint16_t save_sign;
    uint16_t frm_num;
    float    mfcc_dat[vv_frm_max*mfcc_num]; //特征向量N*12
}rt_ftv_arg ;


void cal_mfcc_ftv(rt_active_voice *valid, rt_ftv_arg *v_ftv, rt_atap_arg *atap_arg);

#endif
