#ifndef _MATCH_H

#define _MATCH_H

#include "debug.h"
#include <calcmfcc.h>

#define MATCH_ERR 0xFFFFFFFF
#define MATCH_MAX 0xFFFFFFFF



uint32_t cal_match_dis(rt_ftv_arg *ftv_in, rt_ftv_arg *ftv_mdl);
#endif



