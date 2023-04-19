/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/09/09
* Description        : Main program body.
*******************************************************************************/
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include "drivers/watchdog.h"
#include "drv_lcd.h"
#include <board.h>
#include <calcmfcc.h>
#include <chipflash.h>
#include <activedetect.h>
#include <Get_Data.h>
#include <logo.h>
#include <match.h>
#include "at.h"


/**********************************
    注意rt-thread无法打印浮点，可打开
    编译器的nano_printf选项，代码会大13KB+
**********************************/

/* Global define */
#define VAD_fail    1
#define MFCC_fail   2

#define RCG_ERR         0xFF
#define RCG_EOK         0x00
#define Valid_Thl       245  //有效匹配放大后的距离应小于该值

#define up    0
#define down  1
#define left  2
#define right 3
#define on    4
#define off   5
#define speedup 6
#define speeddown 7


typedef struct
{
    rt_uint8_t result;
    rt_uint8_t dir;
}result_t;


/* Global Variable */
const char *key_words[]={"上", "下", "左", "右","启动","停止","加速","减速" };

rt_uint16_t         V_Data[VDBUF_LEN*2]={1}; //放到data段,放bss不行，待确认
rt_atap_arg         atap_arg;
rt_active_voice     active_voice[max_vc_con];

__attribute__((aligned(4))) rt_ftv_arg   ftv_arg;
struct rt_mailbox  voice_rec_mb;
static rt_uint32_t voice_rcg_mb_pool[4];
struct rt_mailbox  display_mb;
static rt_uint32_t display_mb_pool[4];
static rt_mutex_t  V_Data_mutex=RT_NULL;
result_t rcg_result;

ALIGN(RT_ALIGN_SIZE)
static char recongnition_stack[2048];
static struct rt_thread recongnition_thread;

ALIGN(RT_ALIGN_SIZE)
static char display_stack[2048];
static struct rt_thread display_thread;


/* 触发采集 */
void voice_record(void)
{
#ifdef BSP_USING_ES8388

    I2S_Cmd(SPI2,ENABLE);
    rt_thread_mdelay(start_delay);
    DMA_Rx_Init( DMA1_Channel4, (u32)&SPI2->DATAR, (u32)V_Data, (VDBUF_LEN*2) );
    DMA_Cmd( DMA1_Channel4, ENABLE );

    rt_thread_mdelay(Noise_len_t); //空菜时间段
    lcd_fill(81,0,239,60,WHITE);
    lcd_set_color(WHITE, BLUE);
    lcd_show_string(90,7,24,"speaking...");

#else
    rt_thread_mdelay(20);
    TIM_Cmd(TIM1, ENABLE);
    rt_thread_mdelay(Noise_len_t);

    lcd_fill(81,0,239,60,WHITE);
    lcd_set_color(WHITE, BLUE);
    lcd_show_string(90,7,24,"speaking...");
#endif
}

/* mel模板保存 */
rt_uint8_t save_mdl(rt_uint16_t *v_dat, rt_uint32_t addr)
{
    environment_noise(v_dat,Noise_len,&atap_arg);
    active_detect(v_dat, VDBUF_LEN, active_voice, &atap_arg);
    if(active_voice[0].end==((void *)0))
    {
        return VAD_fail;
    }
    cal_mfcc_ftv(&(active_voice[0]),&ftv_arg,&atap_arg);
    if(ftv_arg.frm_num==0)
    {
        return MFCC_fail;
    }
    return save_ftv_mdl(&ftv_arg, addr);
}

void ch9141_send(rt_uint8_t *data, rt_uint16_t len);

/* 语音识别线程入口函数  */
void voice_recongnition_entry(void *parameter)
{
    rt_uint16_t i=0,min_comm=0;
    rt_uint32_t match_dis,ftvmd_addr,cur_dis,min_dis=0;
    rt_ftv_arg *ftv_mdl;
    rt_uint32_t buffer=0;        //值为V_Data的地址，可用可不用

    while(1)
    {
        rt_mutex_take(V_Data_mutex, RT_WAITING_FOREVER);
        /* 采集触发 */
        voice_record();
        if (rt_mb_recv(&voice_rec_mb, (rt_ubase_t *)&buffer, RT_WAITING_FOREVER) == RT_EOK)//传递V_Data，全局变量随便用哪个
        {

            lcd_set_color(WHITE, RED);
            lcd_fill(81,0,239,60,WHITE);
            lcd_show_string(90,7,24,"stop.");

            environment_noise(V_Data, Noise_len, &atap_arg);
            active_detect(V_Data, VDBUF_LEN, active_voice, &atap_arg);
            if(active_voice[0].end==((void *)0))
            {
                match_dis=MATCH_ERR;
            }
            cal_mfcc_ftv(&(active_voice[0]),&ftv_arg,&atap_arg);
            if(ftv_arg.frm_num==0)
            {
                match_dis=MATCH_ERR;
            }

            i=0;
            min_dis=MATCH_MAX;
            for(ftvmd_addr=ftvmd_start_addr; ftvmd_addr<ftvmd_end_addr; ftvmd_addr+=size_per_ftv)
            {
                /* 模板按rt_ftv_arg存储，直接获取首地址 */
                ftv_mdl=(rt_ftv_arg*)ftvmd_addr;
                cur_dis=((ftv_mdl->save_sign)==SAVE_FLAG)?cal_match_dis(&ftv_arg,ftv_mdl):MATCH_ERR;
                if(cur_dis<min_dis)
                {
                    min_dis=cur_dis;
                    min_comm=i;
                }
                i++;
            }
            min_comm = min_comm/ftvmd_per_word;
            if(min_dis>=Valid_Thl) match_dis=MATCH_ERR;  // 较大时认为不可信，识别失败

            if(match_dis==MATCH_ERR)
            {
                match_dis=0;
                rcg_result.dir=0xFF;
                rcg_result.result=RCG_ERR;
            }
            else
            {
                rcg_result.dir=min_comm;
                rcg_result.result=RCG_EOK;
//                rt_kprintf("recg end,your speaking is: %s\r\n",key_words[min_comm]);
            }
            rt_mb_send(&display_mb, (rt_ubase_t)&rcg_result); //结果发生给显示
        }
        rt_mutex_release(V_Data_mutex);
//        rt_thread_delay(1000);
     }

}


void lcd_display(void *parameter)
{
    rt_int16_t x1=120,y1=150,x2,y2;  //红色小块初始坐标
    rt_uint32_t result=0;
    rt_uint8_t bledata[4]={0};
    x2=x1+10;
    y2=y1+10;

    lcd_clear(WHITE);
    /* 显示logo,log可自己导入 */
    lcd_show_image(0, 0, 80, 60, gImage_1);

    /* 显示红色小块活动范围框 */
    lcd_set_color(WHITE, BLUE);
    lcd_draw_rectangle(3,63,233,233);
    lcd_draw_rectangle(4,64,234,234);
    lcd_draw_rectangle(5,65,235,235);

    /* 显示红色小块初始位置  */
    lcd_fill(x1,y1,x2,y2,RED);

    while(1)
    {
        if (rt_mb_recv(&display_mb, (rt_ubase_t *)&result, RT_WAITING_FOREVER) == RT_EOK)
        {
            if(rcg_result.result == RCG_ERR)
            {
                lcd_show_string(90,7+24,24,"result:xxx");
            }
            else
            {
                switch(rcg_result.dir)
                {
                    case up:
                        lcd_show_string(90,7+24,24,"result:Up");
                        lcd_fill(x1,y1,x2,y2,WHITE);
                        y1-=20;
                        if(y1<75) y1=220;
                        x2=x1+10;
                        y2=y1+10;
                        lcd_fill(x1,y1,x2,y2,RED);
                        break;
                    case down:
                        lcd_show_string(90,7+24,24,"result:Down");
                        lcd_fill(x1,y1,x2,y2,WHITE);
                        y1+=20;
                        if(y1>220) y1=75;
                        x2=x1+10;
                        y2=y1+10;
                        lcd_fill(x1,y1,x2,y2,RED);
                        break;
                    case left:
                        lcd_show_string(90,7+24,24,"result:Left");
                        lcd_fill(x1,y1,x2,y2,WHITE);
                        x1-=20;
                        if(x1<20) x1=220;
                        x2=x1+10;
                        y2=y1+10;
                        lcd_fill(x1,y1,x2,y2,RED);
                        break;
                    case right:
                        lcd_show_string(90,7+24,24,"result:Right");
                        lcd_fill(x1,y1,x2,y2,WHITE);
                        x1+=20;
                        if(x1>225) x1=10;
                        x2=x1+10;
                        y2=y1+10;
                        lcd_fill(x1,y1,x2,y2,RED);
                        break;
                    case on:
                        lcd_show_string(90,7+24,24,"power on");
                        bledata[0]=0xaa;
                        bledata[1]=0x55;
                        bledata[2]=0x01;
                        bledata[3]=0x01;
                        ch9141_send(bledata,4);
                        break;
                    case off:
                        lcd_show_string(90,7+24,24,"power off");
                        bledata[0]=0xaa;
                        bledata[1]=0x55;
                        bledata[2]=0x01;
                        bledata[3]=0x00;
                        ch9141_send(bledata,4);
                        break;
                    case speedup:
                        lcd_show_string(90,7+24,24,"speed up");
                        bledata[0]=0xaa;
                        bledata[1]=0x55;
                        bledata[2]=0x02;
                        bledata[3]=0x01;
                        ch9141_send(bledata,4);
                        break;
                    case speeddown:
                        lcd_show_string(90,7+24,24,"speed dwn");
                        bledata[0]=0xaa;
                        bledata[1]=0x55;
                        bledata[2]=0x02;
                        bledata[3]=0x00;
                        ch9141_send(bledata,4);
                        break;
                    default:
                        break;
                }

            }
        }
    }
}



int main(void)
{
    rt_err_t result;
    rt_uint8_t flash_sta;

    rt_kprintf("MCU: CH32V307\n");
	rt_kprintf("SysClk: %dHz\n",SystemCoreClock);
    rt_kprintf("www.wch.cn\n");

    /* at_client 使用uart7 */
//    at_client_init("uart7",512);

    /* 训练和正常采集不能同时进行  */
    V_Data_mutex = rt_mutex_create("vcbmtx", RT_IPC_FLAG_PRIO);
    if (V_Data_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }
    /* 采集完成通知识别线程 */
    result = rt_mb_init(&voice_rec_mb,
                        "vrecmb",
                        &voice_rcg_mb_pool[0],
                        sizeof(voice_rcg_mb_pool) / 4,
                        RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init vrecmb failed.\n");
        return -1;
    }
    /* 识别完成通知显示线程 */
    result = rt_mb_init(&display_mb,
                        "lcdmb",
                        &display_mb_pool[0],
                        sizeof(display_mb_pool) / 4,
                        RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init lcdmb failed.\n");
        return -1;
    }

    rt_thread_init(&recongnition_thread,
                   "vrcg",
                   voice_recongnition_entry,
                   RT_NULL,
                   &recongnition_stack[0],
                   sizeof(recongnition_stack),
                   4, 20);
    rt_thread_startup(&recongnition_thread);

    rt_thread_init(&display_thread,
                   "display",
                   lcd_display,
                   RT_NULL,
                   &display_stack[0],
                   sizeof(display_stack),
                   10, 20);
    rt_thread_startup(&display_thread);


}



/* 模板训练，导入到fish命令行，根据命令行提示训练和导入模板 */
int practice(void)
{
    rt_uint32_t buffer=0,i=0;
    rt_uint8_t  retry_cnt=0;
    rt_uint32_t addr;


    rt_mutex_take(V_Data_mutex, RT_WAITING_FOREVER); //buffer 互斥
    while(1)
    {

        rt_kprintf("\r\npractice start...\r\n\r\n");

        for (i = 0; i < word_num*ftvmd_per_word; i++) //4位单词个数
        {
            retry:
            printf("please speak:%s \r\n",key_words[i/ftvmd_per_word]);
            voice_record();
            addr=ftvmd_start_addr+(i/ftvmd_per_word)*size_per_word+(i%ftvmd_per_word)*size_per_ftv;
            rt_kprintf("addr:%08x\r\n",addr);

            if (rt_mb_recv(&voice_rec_mb, (rt_ubase_t *)&buffer, RT_WAITING_FOREVER) == RT_EOK)//等待DMA传输完成
            {
                if(save_mdl(V_Data,addr)==PRG_SUCCESS)
                {
                    printf("\r\n %s practice success\r\n",key_words[i/ftvmd_per_word]);
                }
                else
                {
                    printf("%s practice fail !!!\r\n",key_words[i/ftvmd_per_word]);
                    retry_cnt++;
                    if(retry_cnt<5)
                    goto retry;
                    else break;
                }
            }
            Hal_Delay_Ms(1000);//硬件延迟，整个循环只在需要的时候执行一遍，不需要切换
            Hal_Delay_Ms(1000);
        }
        rt_kprintf("practice end!!!\r\n");
        break;
    }

    rt_mutex_release(V_Data_mutex);
    return 0;
}
MSH_CMD_EXPORT(practice, practice voice modle and store in flash);







