/**
 * 
 * 赤菟 v1.1.1 I2C 设备例程。
 * 仅支持 赤菟 v1.1 硬件
 * 磁力计 代码有待继续更新。
 * 
 */
#include "QMI8658.h"
#include "debug.h"
#include "AHT_10.h"
#include "lcd.h"
#include "AP3216C.h"
#include "MPU6050.h"
#include "IIC.h"

/*******************************************************************************
 * Function Name  : main
 * Description    : Main program.
 * Input          : None
 * Return         : None
 *******************************************************************************/
int main(void)
{
	u16 ir, als, ps;
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n", SystemCoreClock);

	lcd_init();

	lcd_set_color(BLACK, WHITE);
	lcd_show_string(50, 0, 32, "openCH.io");
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
	qmi8658_init(); // 初始化 QMI8658，并校准，耗时 2~3 秒
	qmi8658_enable_no_motion(ENABLE,qmi8658_Int_none);

	//  end QMI8658

	// QMC7983
	// 测试 QMC7983 是否存在
	uint8_t i2c_addr = 0X2C;
	uint8_t res = 0x80;
	res = IIC_WriteLen(i2c_addr, 0x0A, 1, &res);
	if (res == 0)
	{
		// 初始化 QMC7983
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

	/* Read samples in polling mode (no int) */
	while (1)
	{
		static float acceleration_mg[3];
		static float angular_rate_mdps[3];
		static float temperature_degC;
		static int16_t emm_lsb[3] = {0};

		qmi8658_read_sensor_data(acceleration_mg, angular_rate_mdps);
		temperature_degC = qmi8658_readTemp();

		lcd_set_color(BLACK, WHITE);
		lcd_show_string(0, 112, 16, "QMI8658  temp : %3d.%2d", (int)temperature_degC, (int)(temperature_degC * 100) - 100 * (int)temperature_degC);
		lcd_set_color(BLACK, GREEN);
		lcd_show_string(30, 128, 16, "X : %6d", (int)(acceleration_mg[0] * 1000));
		lcd_show_string(30, 144, 16, "Y : %6d", (int)(acceleration_mg[1] * 1000));
		lcd_show_string(30, 160, 16, "Z : %6d", (int)(acceleration_mg[2] * 1000));
		lcd_show_string(128, 128, 16, "GX : %6d", (int)(angular_rate_mdps[0] * 1000));
		lcd_show_string(128, 144, 16, "GY : %6d", (int)(angular_rate_mdps[1] * 1000));
		lcd_show_string(128, 160, 16, "GZ : %6d", (int)(angular_rate_mdps[2] * 1000));

		// QMC7983
		i2c_addr = 0x2C;
		u8 display_y = 176;
		u8 emm_status = IIC_ReadByte(i2c_addr, 0x06);
		// Check DRDY
		if (emm_status & 0x01)
		{ 
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
			AP3216C_ReadData(&ir, &ps, &als); // 读取数据
			lcd_set_color(BLACK, WHITE);
			lcd_show_string(0, 48, 16, "AP3216C");
			lcd_set_color(BLACK, GREEN);
			lcd_show_string(30, 64, 16, "IR : %5d", ir);  // IR
			lcd_show_string(30, 80, 16, "PS : %5d", ps);  // PS
			lcd_show_string(30, 96, 16, "ALS: %5d", als); // ALS
		}
	}
}
