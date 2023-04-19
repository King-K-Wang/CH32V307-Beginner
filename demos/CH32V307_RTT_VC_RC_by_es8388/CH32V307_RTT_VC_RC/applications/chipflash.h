#ifndef _CHIPFLASH_H
#define _CHIPFLASH_H

#include "debug.h"
#include <calcmfcc.h>


#define FLASH_PAGE_SIZE     256
#define PRG_FAIL            3
#define PRG_SUCCESS         0

#define SAVE_FLAG           54321
#define size_per_ftv        (8*1024)                            //每个特征向量存储大小，浮点四字节存储
#define page_per_ftv        (size_per_ftv/FLASH_PAGE_SIZE)      //每个特征向量存储占用flash页数
#define ftvmd_per_word      2                                   //每个短词特征模板个数
#define size_per_word       (ftvmd_per_word*size_per_ftv)       //每个短词库占用存储空间大小
#define word_num            8                                   //短词个数
#define ftvmd_total_size    (size_per_word*word_num)            //库占用存储空间大小
#define ftvmd_end_addr      0x08070800                          //模板存储地址下线
#define ftvmd_start_addr    (ftvmd_end_addr-ftvmd_total_size)   //模板存储地址上线，不要与程序空间重合

uint8_t save_ftv_mdl(rt_ftv_arg* ftv, uint32_t addr);

#endif

