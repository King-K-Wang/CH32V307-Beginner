/*******************************************************************************
 * WiFi 模块接口测试例程
 * 
 * ！记得修改 Wi-Fi 名称和密码，或者用例程中的名称和密码开一个热点！
 * 
 * 赤菟开发板上 UART6 连接 WiFi 模块接口，兼容 ESP-01，ESP-01S WiFi 模块
 * 使用时注意 WiFi 天线朝向板外
 * 本例程演示使用 DMA 通过 UART6 与 WiFi 模块通信，访问 api.k780.com 获取时间
 *
 * 例程测试使用 1471 号固件的 ESP-01S，其它版本固件可能有 AT 指令不兼容的情况。
 * ESP-01 的固件与使用说明见安信可官网：https://docs.ai-thinker.com/%E5%9B%BA%E4%BB%B6%E6%B1%87%E6%80%BB
 *
 * 例程中 uartWriteWiFi(), uartWriteWiFiStr() 是非阻塞的。
 * 调用这些函数发送时，若上一次发送尚未完成，将不等待而直接返回，数据不会被发送
 *******************************************************************************/

#include "debug.h"
#include "string.h"

/* Wi-Fi 名称 和 密码 */
#define SSID ("myWi-Fi")
#define PASSWORD ("01234567")

/* Global define */
#define RXBUF_SIZE 1024 // DMA buffer size
#define size(a) (sizeof(a) / sizeof(*(a)))
/* Global Variable */
u8 TxBuffer[] = " ";
u8 RxBuffer[RXBUF_SIZE] = {0};

/*******************************************************************************
 * Function Name  : USARTx_CFG
 * Description    : Initializes the USART6 peripheral.
 * 描述	：	串口初始化
 * Input          : None
 * Return         : None
 *******************************************************************************/
void USARTx_CFG(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	// 开启时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART6, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/* USART6 TX-->C0  RX-->C1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // RX，输入上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;									// 波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 数据位 8
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 停止位 1
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;					// 使能 RX 和 TX

	USART_Init(UART6, &USART_InitStructure);
	DMA_Cmd(DMA2_Channel7, ENABLE); // 开启接收 DMA
	USART_Cmd(UART6, ENABLE);		// 开启UART
}

/*******************************************************************************
 * Function Name  : DMA_INIT
 * Description    : Configures the DMA.
 * 描述	：	DMA 初始化
 * Input          : None
 * Return         : None
 *******************************************************************************/
void DMA_INIT(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

	// TX DMA 初始化
	DMA_DeInit(DMA2_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART6->DATAR);		// DMA 外设基址，需指向对应的外设
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;					// DMA 内存基址，指向发送缓冲区的首地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// 方向 : 外设 作为 终点，即 内存 ->  外设
	DMA_InitStructure.DMA_BufferSize = 0;									// 缓冲区大小,即要DMA发送的数据长度,目前没有数据可发
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址自增，禁用
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增，启用
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据位宽，8位(Byte)
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据位宽，8位(Byte)
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// 普通模式，发完结束，不循环发送
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;					// 优先级最高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// M2P,禁用M2M
	DMA_Init(DMA2_Channel6, &DMA_InitStructure);

	// RX DMA 初始化，环形缓冲区自动接收
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)RxBuffer; // 接收缓冲区
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	  // 方向 : 外设 作为 源，即 内存 <- 外设
	DMA_InitStructure.DMA_BufferSize = RXBUF_SIZE;		  // 缓冲区长度为 RXBUF_SIZE
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;		  // 循环模式，构成环形缓冲区
	DMA_Init(DMA2_Channel7, &DMA_InitStructure);
}

/*******************************************************************************
 * Function Name  :  uartWriteWiFi
 * Description    :  send data to ESP8266 via UART6
 * 描述	：	向 WiFi 模组发送数据
 * Input          :  char * data          data to send	要发送的数据的首地址
 *                   uint16_t num         number of data	数据长度
 * Return         :  RESET                UART6 busy,failed to send	发送失败
 *                   SET                  send success				发送成功
 *******************************************************************************/
FlagStatus uartWriteWiFi(char *data, uint16_t num)
{
	// 如上次发送未完成，返回
	if (DMA_GetCurrDataCounter(DMA2_Channel6) != 0)
	{
		return RESET;
	}

	DMA_ClearFlag(DMA2_FLAG_TC8);
	DMA_Cmd(DMA2_Channel6, DISABLE);			// 关 DMA 后操作
	DMA2_Channel6->MADDR = (uint32_t)data;		// 发送缓冲区为 data
	DMA_SetCurrDataCounter(DMA2_Channel6, num); // 设置缓冲区长度
	DMA_Cmd(DMA2_Channel6, ENABLE);				// 开 DMA
	return SET;
}

/*******************************************************************************
 * Function Name  :  uartWriteWiFiStr
 * Description    :  send string to ESP8266 via UART6	向 WiFi 模组发送字符串
 * Input          :  char * str          string to send
 * Return         :  RESET                UART busy,failed to send	发送失败
 *                   SET                  send success				发送成功
 *******************************************************************************/
FlagStatus uartWriteWiFiStr(char *str)
{
	uint16_t num = 0;
	while (str[num])
		num++; // 计算字符串长度
	return uartWriteWiFi(str, num);
}

/*******************************************************************************
 * Function Name  :  uartReadWiFireceive
 * Description    :  read some bytes from receive buffer 从接收缓冲区读出一组数据
 * Input          :  char * buffer        buffer to storage the data	用来存放读出数据的地址
 *                   uint16_t num         number of data to read		要读的字节数
 * Return         :  int                  number of bytes read		返回实际读出的字节数
 *******************************************************************************/
uint16_t rxBufferReadPos = 0; // 接收缓冲区读指针
uint32_t uartReadWiFi(char *buffer, uint16_t num)
{
	uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7); // 计算 DMA 数据尾的位置
	uint16_t i = 0;
	if (rxBufferReadPos == rxBufferEnd)
	{
		// 无数据，返回
		return 0;
	}

	while (rxBufferReadPos != rxBufferEnd && i < num)
	{
		buffer[i] = RxBuffer[rxBufferReadPos];
		i++;
		rxBufferReadPos++;
		if (rxBufferReadPos >= RXBUF_SIZE)
		{
			// 超出缓冲区，回零
			rxBufferReadPos = 0;
		}
	}
	return i;
}

/*******************************************************************************
 * Function Name  :  uartReadByteWiFi
 * Description    :  read one byte from UART buffer	从接收缓冲区读出 1 字节数据
 * Input          :  None
 * Return         :  char    read data				返回读出的数据(无数据也返回0)
 *******************************************************************************/
char uartReadByteWiFi()
{
	char ret;
	uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);
	if (rxBufferReadPos == rxBufferEnd)
	{
		// 无数据，返回
		return 0;
	}
	ret = RxBuffer[rxBufferReadPos];
	rxBufferReadPos++;
	if (rxBufferReadPos >= RXBUF_SIZE)
	{
		// 超出缓冲区，回零
		rxBufferReadPos = 0;
	}
	return ret;
}
/*******************************************************************************
 * Function Name  :  uartAvailableWiFi
 * Description    :  get number of bytes Available to read from the UART buffer	获取缓冲区中可读数据的数量
 * Input          :  None
 * Return         :  uint16_t    number of bytes Available to read				返回可读数据数量
 *******************************************************************************/
uint16_t uartAvailableWiFi()
{
	uint16_t rxBufferEnd = RXBUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7); // 计算 DMA 数据尾的位置
	// 计算可读字节
	if (rxBufferReadPos <= rxBufferEnd)
	{
		return rxBufferEnd - rxBufferReadPos;
	}
	else
	{
		return rxBufferEnd + RXBUF_SIZE - rxBufferReadPos;
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

	char buffer[1024];
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_Init();
	USART_Printf_Init(115200);
	printf("SystemClk:%d\r\n", SystemCoreClock);

	printf("8266 WiFi TEST\r\n");

	DMA_INIT();
	USARTx_CFG(); /* USART INIT */
	USART_DMACmd(UART6, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

	Delay_Ms(100);
	// 退出透传
	while (uartWriteWiFi("+++", 3));
	Delay_Ms(100);
	// 复位
	while (uartWriteWiFiStr("AT+RST\r\n") == RESET);
	Delay_Ms(1000);

	// 打印之前收到的信息
	int num = uartAvailableWiFi();
	if (num > 0)
	{
		uartReadWiFi(buffer, num);
		buffer[num] = '\0';
		printf("\r\nRevceived %d bytes :\r\n%s", num, buffer);
	}

	// 查询 WiFi 模块是否正常工作
	while (uartWriteWiFi("AT\r\n", 4));
	Delay_Ms(100);
	// 查询 WiFi 模块版本信息
	while (uartWriteWiFi("AT+GMR\r\n", 8) == RESET);
	Delay_Ms(100);
	// 查询模块支持的 AT 命令，一些模块不支持此指令
	while (uartWriteWiFi("AT+CMD\r\n", 8) == RESET);
	Delay_Ms(100);
	// 设为 Station 模式
	while (uartWriteWiFiStr("AT+CWMODE=1\r\n") == RESET);
	Delay_Ms(100);
	// 设为单连接模式
	while (uartWriteWiFiStr("AT+CIPMUX=0\r\n") == RESET);
	Delay_Ms(100);
	// 连接一个名为 SSID、密码为 PASSWORD 的 WiFi 网络，
	while (uartWriteWiFiStr("AT+CWJAP=\"") == RESET);
	while (uartWriteWiFiStr(SSID) == RESET);
	while (uartWriteWiFiStr("\",\"") == RESET);
	while (uartWriteWiFiStr(PASSWORD) == RESET);
	while (uartWriteWiFiStr("\"\r\n") == RESET);
	Delay_Ms(100);

	// 打印之前收到的信息
	num = uartAvailableWiFi();
	if (num > 0)
	{
		uartReadWiFi(buffer, num);
		buffer[num] = '\0';
		printf("\r\nRevceived %d bytes :\r\n%s", num, buffer);
	}

	// 等待 Wi-Fi 连接的回复
	while (uartAvailableWiFi() == 0);
	Delay_Ms(2000);
	num = uartAvailableWiFi();
	if (num > 0)
	{
		uartReadWiFi(buffer, num);
		buffer[num] = '\0';
		printf("\r\nRevceived %d bytes :\r\n%s", num, buffer);
	}

	// 连接服务器
	while (uartWriteWiFiStr("AT+CIPSTART=\"TCP\",\"api.k780.com\",80\r\n") == RESET);
	Delay_Ms(500);
	// 进入透传
	while (uartWriteWiFiStr("AT+CIPMODE=1\r\n") == RESET);
	Delay_Ms(100);
	while (uartWriteWiFiStr("AT+CIPSEND\r\n") == RESET);
	Delay_Ms(100);

	num = uartAvailableWiFi();
	if (num > 0)
	{
		uartReadWiFi(buffer, num);
		buffer[num] = '\0';
		printf("\r\nRevceived %d bytes :\r\n%s", num, buffer);
	}

	// 获取时间
	while (uartWriteWiFiStr("GET http://api.k780.com:88/?app=life.time&appkey=10003&sign=b59bc3ef6191eb9f747dd4e83c99f2a4&format=json\r\n") == RESET);
	Delay_Ms(500);

	num = uartAvailableWiFi();
	if (num > 0)
	{
		uartReadWiFi(buffer, num);
		buffer[num] = '\0';
		printf("\r\nRevceived %d bytes :\r\n%s\r\n", num, buffer);
	}

	while (1)
	{
	}
}
