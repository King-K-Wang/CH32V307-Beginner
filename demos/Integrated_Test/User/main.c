/**
 * 
 * 赤菟 v1.1.1 综合测试程序。
 * 仅支持 赤菟 v1.1 硬件
 * 磁力计 代码有待继续更新。
 * 
 */
#include "QMI8658.h"
#include "IIC.h"
#include "debug.h"
#include "es8388.h"
#include "AHT_10.h"
#include "lcd.h"
#include "AP3216C.h"
#include "MPU6050.h"

// flash
/* FLASH Keys */
#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)

/* Delay definition */
#define ProgramTimeout ((uint32_t)0x00005000)
//! flash

/* Global Variable */

#define up 1
#define down 2
#define left 3
#define right 4
#define sel 5
#define sw1 6
#define sw2 7
#define wakeup 8

#define LEN 64000 // 在SRAM中以16位形式存放数据长度。8K采样率，双通道，1s数据32KB

u16 RxData[LEN]; // 数据存放的首地址

void GPIO_INIT()
{
	GPIO_InitTypeDef GPIO_InitTypdefStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitTypdefStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitTypdefStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitTypdefStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitTypdefStruct);

	GPIO_InitTypdefStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitTypdefStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitTypdefStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitTypdefStruct);

	GPIO_InitTypdefStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_13;
	GPIO_InitTypdefStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitTypdefStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitTypdefStruct);
}

/*******************************************************************************
 * Function Name  : Basic_Key_Handle
 * Description    : Basic Key Handle
 * Input          : None
 * Return         : 0 = no key press
 *                  1 = key press down
 *******************************************************************************/
uint8_t Basic_Key_Handle(void)
{
	uint8_t keyval = 0;
	if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
	{
		Delay_Ms(20);
		if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4))
		{
			keyval = sw1;
		}
	}
	else
	{
		if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5))
		{
			Delay_Ms(20);
			if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5))
			{
				keyval = sw2;
			}
		}
		else
		{
			if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
			{
				Delay_Ms(20);
				if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
				{
					keyval = wakeup;
				}
			}
			else
			{
				if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1))
				{
					Delay_Ms(20);
					if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1))
					{
						keyval = up;
					}
				}
				else
				{
					if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2))
					{
						Delay_Ms(20);
						if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2))
						{
							keyval = down;
						}
					}
					else
					{
						if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
						{
							Delay_Ms(20);
							if (!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3))
							{
								keyval = right;
							}
						}
						else
						{
							if (!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6))
							{
								Delay_Ms(20);
								if (!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6))
								{
									keyval = left;
								}
							}
							else
							{
								if (!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13))
								{
									Delay_Ms(20);
									if (!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13))
									{
										keyval = sel;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return keyval;
}

void LED_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_11);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_12);
}

// 串口DMA传输8388采集的数据
void DMA_INIT(u16 bufsize)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR); /* USART2->DATAR:0x40004404 */
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxData;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = bufsize; // 串口发送8位数据，传输量是存储长度两倍
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);
}

void I2S2_Init_TX(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	I2S_InitTypeDef I2S_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;

	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;

	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_8k;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_Init(SPI2, &I2S_InitStructure);

	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	I2S_Cmd(SPI2, ENABLE);
}

/*******************************************************************************
 * Function Name  : I2S2_Init
 * Description    : Init I2S2
 * Input          : None
 * Return         : None
 *******************************************************************************/
void I2S2_Init_RX(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2S_InitTypeDef I2S_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
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

	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
	I2S_Cmd(SPI2, ENABLE); // 开启I2SDMA接收8388数据  ，开启I2S就开始接收数据。
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
void DMA_Rx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

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
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA_CHx, &DMA_InitStructure);
}

/*******************************************************************************
 * Function Name  : DMA_Tx_Init
 * Description    : Initializes the I2S2 DMA Channelx configuration.
 * Input          : DMA_CHx:
 *                    x can be 1 to 7.
 *                  ppadr: Peripheral base address.
 *                  memadr: Memory base address.
 *                  bufsize: DMA channel buffer size.
 * Return         : None
 *******************************************************************************/
void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_DeInit(DMA_CHx);

	DMA_InitStructure.DMA_PeripheralBaseAddr = ppadr;
	DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = bufsize;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA_CHx, &DMA_InitStructure);
}

void record(void)
{

	ES8388_ADDA_Cfg(1, 0);
	ES8388_Write_Reg(0x04, 0x00);
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, 1); // 控制PA8，为1设置数据输入307
	Delay_Ms(10);
	printf("Set PA8 1\r\n");
	printf("Init I2S\r\n");
	I2S2_Init_RX(); // I2S2 端口初始化，配置I2S2 通信格式飞利浦，16位，8K  初始化结束后开启I2S2

	Delay_Ms(10);	   // 等待8388返回数据

	DMA_Rx_Init(DMA1_Channel4, (u32)&SPI2->DATAR, (u32)RxData, LEN / 2); // 初始化I2S2 DMA，接收8388返回数据
	DMA_Cmd(DMA1_Channel4, ENABLE);										 // 开启I2S2 DMA

	while ((!DMA_GetFlagStatus(DMA1_FLAG_TC4)))
		; // 等待一次传输完成
	printf("record end!\r\n");
	I2S_Cmd(SPI2, DISABLE); // 关闭I2S2 ，8388停止返回数据
	DMA_DeInit(DMA1_Channel4);
	printf("record out!\r\n");
}

void play(void)
{

	ES8388_ADDA_Cfg(0, 1);
	ES8388_Output_Cfg(1);
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, 0); // 控制PA8，为1设置数据输入307

	printf("Set PA8 0\r\n");
	Delay_Ms(10);
	printf("Init I2S\r\n");
	I2S2_Init_TX();

	DMA_Tx_Init(DMA1_Channel5, (u32)&SPI2->DATAR, (u32)RxData, LEN / 2);
	DMA_Cmd(DMA1_Channel5, ENABLE);

	while ((!DMA_GetFlagStatus(DMA1_FLAG_TC5)));

	printf("play end!\r\n");
	I2S_Cmd(SPI2, DISABLE); // 关闭I2S2 ，8388停止返回数据
	DMA_DeInit(DMA1_Channel5);
	printf("play out!\r\n");
}

void test_sensor()
{
	u16 ir, als, ps;
	u8 display_y;
	lcd_set_color(BLACK, WHITE);
	lcd_show_string(72, 0, 32, "openCH");
	lcd_set_color(BLACK, RED);
	lcd_show_string(112, 32, 16, "I2C Device Test");

	static float acceleration_mg[3];
	static float angular_rate_mdps[3];
	static float temperature_degC;
	static int16_t emm_lsb[3];

	qmi8658_read_sensor_data(acceleration_mg, angular_rate_mdps);
	temperature_degC = qmi8658_readTemp();
	display_y = 96;
	lcd_set_color(BLACK, WHITE);
	lcd_show_string(0, display_y, 16, "QMI8658");
	lcd_set_color(BLACK, YELLOW);
	lcd_show_string(72, display_y, 16, "temp : %3d.%2d", (int)temperature_degC, (int)(temperature_degC * 100) - 100 * (int)temperature_degC);
	lcd_set_color(BLACK, GREEN);
	lcd_show_string(30, display_y + 16, 16, "X : %6d", (int)(acceleration_mg[0] * 1000));
	lcd_show_string(30, display_y + 32, 16, "Y : %6d", (int)(acceleration_mg[1] * 1000));
	lcd_show_string(30, display_y + 48, 16, "Z : %6d", (int)(acceleration_mg[2] * 1000));
	lcd_show_string(128, display_y + 16, 16, "GX : %6d", (int)(angular_rate_mdps[0] * 1000));
	lcd_show_string(128, display_y + 32, 16, "GY : %6d", (int)(angular_rate_mdps[1] * 1000));
	lcd_show_string(128, display_y + 48, 16, "GZ : %6d", (int)(angular_rate_mdps[2] * 1000));

	// QMC7983
	u8 i2c_addr = 0x2C;
	display_y = 160;
	u8 emm_status = IIC_ReadByte(i2c_addr, 0x06);
	if (emm_status & 0x01)
	{ // Check DRDY
		IIC_ReadLen(i2c_addr, 0x00, 6, (u8 *)emm_lsb);
		lcd_set_color(BLACK, WHITE);
		lcd_show_string(0, display_y, 16, "QMC7983");
		lcd_set_color(BLACK, GREEN);
		lcd_show_string(30, display_y + 16, 16, "X : %6d", emm_lsb[0]);
		lcd_show_string(30, display_y + 32, 16, "Y : %6d", emm_lsb[1]);
		lcd_show_string(30, display_y + 48, 16, "Z : %6d", emm_lsb[2]);
	}
	// END QMC7983
	{ // AP3216C
		// printf("read AP3216\r\n");
		display_y = 32;
		AP3216C_ReadData(&ir, &ps, &als); // 读取数据
		lcd_set_color(BLACK, WHITE);
		lcd_show_string(0, display_y, 16, "AP3216C");
		display_y += 16;
		lcd_set_color(BLACK, GREEN);
		lcd_show_string(30, display_y, 16, "IR : %5d", ir); // IR
		display_y += 16;
		lcd_show_string(30, display_y, 16, "PS : %5d", ps); // PS
		display_y += 16;
		lcd_show_string(30, display_y, 16, "ALS: %5d", als); // ALS
		display_y += 16;
	}

	lcd_set_color(BLACK, GBLUE);
	lcd_show_string(10, 224, 16, "SW1:PRE");
	lcd_show_string(130, 224, 16, "SW2:NEXT");
	Delay_Ms(100);
}

void test_key(void)
{
	lcd_set_color(BLACK, WHITE);
	lcd_show_string(72, 0, 32, "openCH");
	lcd_set_color(BLACK, RED);
	lcd_show_string(0, 32, 32, "Key Test");
	lcd_set_color(BLACK, GBLUE);
	lcd_show_string(0, 200, 24, "SW1:PRE  SW2:NEXT");
	lcd_set_color(BLACK, GBLUE);
	switch (Basic_Key_Handle())
	{
	case up:
		lcd_show_string(80, 80, 32, "   UP  ");
		break;
	case down:
		lcd_show_string(80, 80, 32, " DOWN  ");
		break;
	case left:
		lcd_show_string(80, 80, 32, " LEFT ");
		break;
	case right:
		lcd_show_string(80, 80, 32, "RIGHT ");
		break;
	case sel:
		lcd_show_string(80, 80, 32, "  SEL  ");
		break;
	case wakeup:
		lcd_show_string(80, 80, 32, "WakeUp ");
		break;
	default:
		break;
	}
}

void test_es8388(void)
{
	static u16 PreKey = 0;
	static u16 CurKey = 0;
	PreKey = CurKey;
	CurKey = Basic_Key_Handle();

	lcd_set_color(BLACK, WHITE);
	lcd_show_string(50, 0, 32, "openCH");
	lcd_set_color(BLACK, RED);
	lcd_show_string(0, 32, 24, "ES8388 Test");
	lcd_set_color(BLACK, GBLUE);
	lcd_show_string(16, 64, 24, "Left:Record");
	lcd_show_string(16, 96, 24, "Right:Play");

	if (CurKey != PreKey)
	{
		if (CurKey == left)
		{
			lcd_show_string(48, 160, 24, "Recording...    ");
			record();
			lcd_show_string(48, 160, 24, "Record   END    ");
			printf("Record return!\r\n");
		}
		else
		{
			if (CurKey == right)
			{
				lcd_show_string(48, 160, 24, " Playing...   ");
				play();
				lcd_show_string(48, 160, 24, " Play   END   ");
			}
		}
	}
}

void test_lcd(void)
{
	lcd_fill(0, 0, 239, 239, WHITE);
	Delay_Ms(300);
	lcd_fill(0, 0, 239, 239, BLACK);
	Delay_Ms(300);
	lcd_fill(0, 0, 239, 239, RED);
	Delay_Ms(300);
	lcd_fill(0, 0, 239, 239, GREEN);
	Delay_Ms(300);
	lcd_fill(0, 0, 239, 239, BLUE);
	Delay_Ms(300);
	lcd_clear(BLACK);
}

/**
 * @brief 检查 FLASH 配置
 *
 * @param ramSetting	: 期望的 FLASH 配置代码
 * @return int 	: 实际 FLASH 配置代码
 * FLASH 配置代码：
 * 		0 - 128kB RAM
 * 		1 -  96kB RAM
 * 		2 -  64kB RAM
 * 		3 -  36kB RAM
 */
void checkRamCfg(uint16_t ramSetting)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t Addr = 0x1FFFF800;
	__IO uint8_t i;
	uint16_t pbuf[8];
	uint16_t oldCfg;
	FLASH->OBKEYR = FLASH_KEY1;
	FLASH->OBKEYR = FLASH_KEY2;
	status = FLASH_WaitForLastOperation(ProgramTimeout);
	char *cfgString[4] = {"128kB", "96kB", "64kB", "36kB"};

	if (status == FLASH_COMPLETE)
	{
		/* Read optionbytes */
		for (i = 0; i < 8; i++)
		{
			pbuf[i] = *(uint16_t *)(Addr + 2 * i);
		}
		oldCfg = (pbuf[1] >> 8) & 0x03; // get old SRAM_CODE_MODE:FLASH_OBR[9:8]
		printf("Expected Ram : %s .Actual : %s .\n", cfgString[ramSetting], cfgString[oldCfg]);
	}
}

/*******************************************************************************
 * Function Name  : main
 * Description    : Main program.
 * Input          : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
	int16_t key_count = 0;
	u16 Prekeyvalue = 0;
	u16 Curkeyvalue = 0;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n", SystemCoreClock);
	checkRamCfg(0);
	GPIO_INIT();
	LED_GPIO_Init();
	lcd_init();
	lcd_set_color(BLACK, WHITE);
	lcd_show_string(72, 0, 32, "openCH");
	lcd_set_color(BLACK, RED);
	lcd_show_string(0, 32, 16, "I2C Device Test");
	lcd_set_color(BLACK, WHITE);
	lcd_show_string(0, 48, 16, "AP3216C");
	while (AP3216C_Init()) // 初始化AP3216C
	{
		lcd_set_color(BLACK, RED);
		lcd_show_string(180, 48, 16, "Error");
		Delay_Ms(200);
		lcd_show_string(180, 48, 16, "     ");
		Delay_Ms(200);
	}
	lcd_set_color(BLACK, GREEN);
	lcd_show_string(180, 48, 16, "OK");

	// QMI8658
	qmi8658_init();
	qmi8658_enable_no_motion(ENABLE, qmi8658_Int_none);
	// end QMI8658

	// QMC7983
	uint8_t i2c_addr = 0X2C;
	uint8_t res = 0x80;
	res = IIC_WriteLen(i2c_addr, 0x0A, 1, &res);
	if (res == 0)
	{
		delay_ms(10);
		res = 0x31;
		res = IIC_WriteLen(i2c_addr, 0x09, 1, &res);
		res = IIC_ReadByte(i2c_addr, 0x0D);
		printf("I2C data @ address 0x%2X : 0x%2X\r\n", i2c_addr, res);
	}
	else
	{
		printf("I2C device not found @ address 0x%2X\r\n", i2c_addr);
	}
	// end QMC7983

	printf("Start \r\n");
	printf("Init 8388\r\n");
	ES8388_Init();		   // ES8388初始化
	ES8388_Set_Volume(22); // 设置耳机音量

	ES8388_I2S_Cfg(0, 3);  // 配置为飞利浦格式，16bit数据
	ES8388_ADDA_Cfg(1, 1); // 关闭AD 关闭DA
	ES8388_Input_Cfg(0);   // AD选择1通道

	while (1)
	{
		Prekeyvalue = Curkeyvalue;
		Curkeyvalue = Basic_Key_Handle();

		if (Curkeyvalue != Prekeyvalue)
		{
			if (Curkeyvalue == sw2)
			{
				lcd_clear(BLACK);
				key_count++;
				if (key_count > 3)
				{
					key_count = 3;
				}
			}
			else
			{
				if (Curkeyvalue == sw1)
				{
					lcd_clear(BLACK);
					key_count--;
					if (key_count < 0)
					{
						key_count = 0;
					}
				}
			}
		}
		switch (key_count)
		{
		case 0:
			test_lcd();
			key_count++;
			break;
		case 1:
			test_sensor();
			break;
		case 2:
			test_key();
			break;
		case 3:
			test_es8388();
			break;
		default:
			break;
		}
	}

	while (1)
		;
}
