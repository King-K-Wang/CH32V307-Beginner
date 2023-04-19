#include <Get_Data.h>
#include <rtthread.h>
#include <rthw.h>

#ifdef BSP_USING_ES8388
#include "drv_es8388.h"
#endif

extern rt_uint16_t V_Data[VDBUF_LEN*2];


#ifdef BSP_USING_ES8388
#define CODEC_I2C_NAME   ("i2c2")
void I2S2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2S_InitTypeDef  I2S_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;
    I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
    I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;

    I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
    I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;
    I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;
    I2S_Init(SPI2, &I2S_InitStructure);

    SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Rx, ENABLE);
    I2S_Cmd(SPI2,DISABLE);   //开启I2SDMA接收8388数据  ，开启I2S就开始接收数据。
}


/*******************************************************************************
* Function Name  : DMA_Rx_Init
* Description    : Initializes the I2S2 DMA Channelx configuration.
* Input          : DMA_CHx:
*                    x can be 1 to 7.
*                  ppadr; Peripheral base address.
*                  memadr: Memory base address.
*                  bufsize: DMA channel buffer size.
* Return         : None
*******************************************************************************/
void DMA_Rx_Init( DMA_Channel_TypeDef* DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize )
{
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    DMA_DeInit(DMA_CHx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init( DMA_CHx, &DMA_InitStructure );
    DMA_ITConfig(DMA_CHx,DMA_IT_TC,ENABLE);   //使能传输完成中断
}


void DMA_Data_Tran(DMA_Channel_TypeDef* DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    DMA_DeInit(DMA_CHx);

    DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
    DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = bufsize;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
    DMA_Init( DMA_CHx, &DMA_InitStructure );
    DMA_ITConfig(DMA_CHx,DMA_IT_TC,ENABLE);   //使能传输完成中断
    DMA_Cmd( DMA_CHx, ENABLE );
}


int voice_init(void)
{
    ES8388_Init(CODEC_I2C_NAME,8);                  //pin8--->PA8
    ES8388_Set_Volume(22);                          //设置耳机音量
    ES8388_I2S_Cfg(0,3);                            //配置为飞利浦格式，16bit数据
    ES8388_ADDA_Cfg(1,0);                           //开启AD 关闭DA
    I2S2_Init();
    NVIC_SetPriority(DMA1_Channel4_IRQn,0xE0);     /* 设置优先级 */
    NVIC_EnableIRQ(DMA1_Channel4_IRQn);            /* 使能内核DMA中断 */

    NVIC_SetPriority(DMA1_Channel5_IRQn,0xE0);     /* 设置优先级 */
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);            /* 使能内核DMA中断 */
    rt_kprintf("init es8388\r\n");
    return 0;
}
INIT_ENV_EXPORT(voice_init); //加入自动初始化



void odd_even_exchange(uint16_t *dat, uint16_t len)
{
    uint16_t i=0,j=0,temp=0;
    for(j=1;j<(len/2);j++)
    {
        for(i=j;i<(len-j);i+=2)
        {
            temp=dat[i];
            dat[i]=dat[i+1];
            dat[i+1]=temp;
        }
    }

}

extern struct rt_mailbox  voice_rec_mb;
void DMA1_Channel4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel4_IRQHandler(void)
{
    GET_INT_SP();
    rt_interrupt_enter();

   if(DMA_GetITStatus(DMA1_IT_TC4))
   {
       DMA_ClearITPendingBit(DMA1_IT_TC4|DMA1_IT_GL4|DMA1_IT_HT4); //传输完成、传输过半、全局中断都会置位
       I2S_Cmd(SPI2,DISABLE);
//       rt_kprintf("\r\n");
//       for(int i=0;i<VDBUF_LEN*2;i++)
//       {
//           rt_kprintf("%04x  ",V_Data[i]);
//           if((i%16==0) && (i!=0))
//           {
//               rt_kprintf("\r\n");
//           }
//       }
//       rt_kprintf("\r\n=====================================================\r\n");
       DMA_Data_Tran(DMA1_Channel5, (u32)V_Data, (u32)V_Data,VDBUF_LEN);
   }

    rt_interrupt_leave();
    FREE_INT_SP();
}


void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void DMA1_Channel5_IRQHandler(void)
{
    GET_INT_SP();
    rt_interrupt_enter();

   if(DMA_GetITStatus(DMA1_IT_TC5))
   {
       DMA_ClearITPendingBit(DMA1_IT_TC5|DMA1_IT_GL5|DMA1_IT_HT5); //传输完成、传输过半、全局中断都会置位
       for(uint16_t i=0;i<VDBUF_LEN;i++)
       {
           V_Data[i]=(uint16_t)((int16_t)V_Data[i]+32768);
       }
       rt_mb_send(&voice_rec_mb, (rt_ubase_t)V_Data);   //发送的是V_Data准备好
   }
    rt_interrupt_leave();
    FREE_INT_SP();
}
#endif



