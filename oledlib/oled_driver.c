/**
 * @file oled_driver.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief 	
 * @version 0.1
 * @date 2021-09-16
 * 
 * @copyright Copyright (c) 2021
 * 
 * @note
 * 	图形库原理:其实就是对一个数组进行操作,数组操作完成之后,直接将整个数组刷新到屏幕上
 * 因此,此c文件用于配置oled底层 用于单片机与oled的直接且唯一通信
 * 移植此图形库主要完善单片机与OLED的通信:
 * SPI_Configuration() 	配置通信引脚
 * WriteCmd()      		写命令
 * WriteDat()      		写数据
 * OledTimeMsFunc()  oled_config中的函数 为系统提供时基
 * 此例程仅演示IIC通信方式 需要更改其他通信方式请在[oled_config.h]修改
 */

#include "oled_driver.h"
#include "Delay.h"
#include "main.h"
#include "stm32f4xx_hal_def.h"

// OLED 初始化指令数组
uint8_t OLED_Init_CMD[] =
	{0xAE, 0x20, 0x10, 0xb0, 0xc8, 0x00, 0x10, 0x40, 0x81, 0xff,
	 0xa1, 0xa6, 0xa8, 0x3F, 0xa4, 0xd3, 0x00, 0xd5, 0xf0, 0xd9,
	 0x22, 0xda, 0x12, 0xdb, 0x20, 0x8d, 0x14, 0xaf};

#if (TRANSFER_METHOD == HW_IIC) || (TRANSFER_METHOD == HW_IIC_DMA) //1.硬件IIC

//...
extern unsigned char ScreenBuffer[SCREEN_PAGE_NUM][SCREEN_COLUMN];

//...

extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_tx;

//I2C_Configuration，初始化硬件IIC引脚, 废弃
// ! 建议在 STM32CubeMX中直接配置
void I2C_Configuration(void)
{
	GPIO_InitTypeDef gpio_init = {0};

	/**
	 * PB6	--> I2C1_SCL
	 * PB7  --> I2C1_SDA
	 * */
	IIC_GPIO_ENABLE();
	__HAL_RCC_I2C1_CLK_ENABLE();

	// GPIO 模式配置
	gpio_init.Pin = IIC_SCL_PIN_X | IIC_SDA_PIN_X;
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_init.Mode = GPIO_MODE_AF_OD;
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(IIC_GPIOX, &gpio_init);

	// * I2C 模式配置
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
#if (TRANSFER_METHOD == HW_IIC_DMA)
	// DMA TX 初始化
	hdma_i2c1_tx.Instance = DMA1_Stream6;
	hdma_i2c1_tx.Init.Channel = DMA_CHANNEL_1;
	hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
	hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_i2c1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK)
	{
		Error_Handler();
	}

	__HAL_LINKDMA(&hi2c1, hdmatx, hdma_i2c1_tx);
#endif
	/* I2C1 interrupt Init */
	HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
}

#if (TRANSFER_METHOD == HW_IIC_DMA)
static void DMA_WriteCmd(uint8_t *buf, int len)
{
	HAL_I2C_Mem_Write_DMA(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteCom_Addr, I2C_MEMADD_SIZE_8BIT, buf, len);
}
static void DMA_WriteDat(uint8_t *buf, int len)
{
	HAL_I2C_Mem_Write_DMA(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteData_Addr, I2C_MEMADD_SIZE_8BIT, buf, len);
}

#else
/**
 * @brief 		I2C_WriteByte，向OLED寄存器地址写一个byte的数据
 * 				如果不使用 HAL 库, 或自行写 I2C 的发送 函数
 * 				则再此重写发送函数
 * 
 * @param addr 	寄存器地址
 * @param data 	要写入的数据
 * 
 * @date 2021-08-28
 */
void I2C_WriteByte(uint8_t addr, uint8_t data)
{
}

void WriteCmd(unsigned char *cmd, int len) //写命令
{
#if USING_DMA
	HAL_I2C_Mem_Write_DMA(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteCom_Addr, I2C_MEMADD_SIZE_8BIT, cmd, len);
#else
	HAL_I2C_Mem_Write(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteCom_Addr, I2C_MEMADD_SIZE_8BIT, cmd, len, 1000);
#endif
}

void WriteDat(unsigned char *dat, int len) //写数据
{
#if USING_DMA
	HAL_I2C_Mem_Write_DMA(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteData_Addr, I2C_MEMADD_SIZE_8BIT, buf, len);
#else
	HAL_I2C_Mem_Write(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteData_Addr, I2C_MEMADD_SIZE_8BIT, dat, len, 1000);
#endif
}
#endif

#elif (TRANSFER_METHOD == SW_IIC) //2.软件IIC

void IIC_Start(void);
void IIC_Send_Byte(uint8_t txd);
void IIC_Stop(void);

//初始化
void I2C_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(OLED_RCC_I2C_PORT, ENABLE); /* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Pin = OLED_I2C_SCL_PIN | OLED_I2C_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; /* 开漏输出 */
	GPIO_Init(OLED_GPIO_PORT_I2C, &GPIO_InitStructure);

	IIC_SDA = 1;
	IIC_SCL = 1;
}

/**
		 @brief  I2C_WriteByte，向OLED寄存器地址写一个byte的数据
		 @param  addr：寄存器地址 data：要写入的数据
		 @retval 无
	*/
void I2C_WriteByte(uint8_t addr, uint8_t data)
{
	IIC_Start();
	IIC_Send_Byte(OLED_ADDRESS);
	IIC_Send_Byte(addr); //write data
	IIC_Send_Byte(data);
	IIC_Stop();
}

void WriteCmd(unsigned char cmd) //写命令
{
	I2C_WriteByte(0x00, cmd);
}

void WriteDat(unsigned char dat) //写数据
{
	I2C_WriteByte(0x40, dat);
}

//以下函数开始为软件IIC驱动
//产生IIC起始信号
void IIC_Start(void)
{
	IIC_SCL = 1;
	IIC_SDA = 1;
	//delay_us(1);
	IIC_SDA = 0;
	//delay_us(1);
	IIC_SCL = 0;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(uint8_t IIC_Byte)
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		if (IIC_Byte & 0x80)
			IIC_SDA = 1;
		else
			IIC_SDA = 0;
		//delay_us(1);
		IIC_SCL = 1;
		delay_us(1); //必须有保持SCL脉冲的延时
		IIC_SCL = 0;
		IIC_Byte <<= 1;
	}
	//原程序这里有一个拉高SDA，根据OLED的要求，此句必须去掉。
	IIC_SCL = 1;
	delay_us(1);
	IIC_SCL = 0;
}

//产生IIC停止信号
void IIC_Stop(void)
{
	IIC_SDA = 0;
	//delay_us(1);
	IIC_SCL = 1;
	//delay_us(1);
	IIC_SDA = 1;
}

#elif (TRANSFER_METHOD == HW_SPI) || (TRANSFER_METHOD == HW_SPI_DMA) //3.硬件SPI

#define OLED_RESET_LOW() GPIO_ResetBits(SPI_RES_GPIOX, SPI_RES_PIN) //低电平复位
#define OLED_RESET_HIGH() GPIO_SetBits(SPI_RES_GPIOX, SPI_RES_PIN)

#define OLED_CMD_MODE() GPIO_ResetBits(SPI_DC_GPIOX, SPI_DC_PIN) //命令模式
#define OLED_DATA_MODE() GPIO_SetBits(SPI_DC_GPIOX, SPI_DC_PIN)	 //数据模式

#define OLED_CS_HIGH() GPIO_SetBits(SPI_CS_GPIOX, SPI_CS_Pin_X)
#define OLED_CS_LOW() GPIO_ResetBits(SPI_CS_GPIOX, SPI_CS_Pin_X)

void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
#if (USE_HW_SPI == SPI_2)
	RCC_APB1PeriphClockCmd(SPI_RCC_APB1Periph_SPIX, ENABLE);
#elif (USE_HW_SPI == SPI_1)
	RCC_APB2PeriphClockCmd(SPI_RCC_APB2Periph_SPIX, ENABLE);
#endif
	RCC_APB2PeriphClockCmd(SPI_RCC_APB2Periph_GPIOX | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SPI_CS_Pin_X;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_CS_GPIOX, &GPIO_InitStructure);
	OLED_CS_HIGH();

	GPIO_InitStructure.GPIO_Pin = SPI_HW_ALL_PINS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPI_HW_ALL_GPIOX, &GPIO_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPIX, &SPI_InitStructure);
	SPI_Cmd(SPIX, ENABLE);

	GPIO_InitStructure.GPIO_Pin = SPI_RES_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_RES_GPIOX, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPI_DC_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_DC_GPIOX, &GPIO_InitStructure);
	OLED_DATA_MODE();

	OLED_RESET_HIGH();
	WaitTimeMs(100);
	OLED_RESET_LOW();
	WaitTimeMs(100);
	OLED_RESET_HIGH();
}

void SPI_WriterByte(unsigned char dat)
{

	while (SPI_I2S_GetFlagStatus(SPIX, SPI_I2S_FLAG_TXE) == RESET)
	{
	};							 //检查指定的SPI标志位设置与否:发送缓存空标志位
	SPI_I2S_SendData(SPIX, dat); //通过外设SPIx发送一个数据
	while (SPI_I2S_GetFlagStatus(SPIX, SPI_I2S_FLAG_RXNE) == RESET)
	{
	};						   //检查指定的SPI标志位设置与否:接受缓存非空标志位
	SPI_I2S_ReceiveData(SPIX); //返回通过SPIx最近接收的数据
}

void WriteCmd(unsigned char cmd)
{
	OLED_CMD_MODE();
	OLED_CS_LOW();
	SPI_WriterByte(cmd);
	OLED_CS_HIGH();
	OLED_DATA_MODE();
}

void WriteDat(unsigned char dat)
{
	OLED_DATA_MODE();
	OLED_CS_LOW();
	SPI_WriterByte(dat);
	OLED_CS_HIGH();
	OLED_DATA_MODE();
}

#elif (TRANSFER_METHOD == SW_SPI) //4.软件SPI

//引脚初始化
void SPI_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能A端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //速度50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_4);

	OLED_RST_Set();
	delay_ms(100);
	OLED_RST_Clr();
	delay_ms(200);
	OLED_RST_Set();
}

//写字节
void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
	uint8_t i;
	if (cmd)
		OLED_DC_Set();
	else
		OLED_DC_Clr();
	OLED_CS_Clr();
	for (i = 0; i < 8; i++)
	{
		OLED_SCLK_Clr();
		if (dat & 0x80)
			OLED_SDIN_Set();
		else
			OLED_SDIN_Clr();
		OLED_SCLK_Set();
		dat <<= 1;
	}
	OLED_CS_Set();
	OLED_DC_Set();
}

//写命令
void WriteCmd(unsigned char cmd)
{
	OLED_WR_Byte(cmd, 0);
}

//写数据
void WriteDat(unsigned char dat)
{
	OLED_WR_Byte(dat, 1);
}

#endif //(TRANSFER_METHOD ==HW_IIC)

void OLED_Init(void)
{
	int length = sizeof(OLED_Init_CMD) / sizeof(OLED_Init_CMD[0]);
#if (TRANSFER_METHOD == HW_SPI_DMA) || (TRANSFER_METHOD == HW_IIC_DMA)
	DMA_WriteCmd(OLED_Init_CMD, length);
#else
	WriteCmd(OLED_Init_CMD, length);

	// WriteCmd(0xAE); //display off
	// WriteCmd(0x20); //Set Memory Addressing Mode
	// WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	// WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
	// WriteCmd(0xc8); //Set COM Output Scan Direction
	// WriteCmd(0x00); //---set low column address
	// WriteCmd(0x10); //---set high column address
	// WriteCmd(0x40); //--set start line address
	// WriteCmd(0x81); //--set contrast control register
	// WriteCmd(0xff); //亮度调节 0x00~0xff
	// WriteCmd(0xa1); //--set segment re-map 0 to 127
	// WriteCmd(0xa6); //--set normal display
	// WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	// WriteCmd(0x3F); //
	// WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	// WriteCmd(0xd3); //-set display offset
	// WriteCmd(0x00); //-not offset
	// WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	// WriteCmd(0xf0); //--set divide ratio
	// WriteCmd(0xd9); //--set pre-charge period
	// WriteCmd(0x22); //
	// WriteCmd(0xda); //--set com pins hardware configuration
	// WriteCmd(0x12);
	// WriteCmd(0xdb); //--set vcomh
	// WriteCmd(0x20); //0x20,0.77xVcc
	// WriteCmd(0x8d); //--set DC-DC enable
	// WriteCmd(0x14); //
	// WriteCmd(0xaf); //--turn on oled panel
#endif
	OLED_CLS();
}

void OLED_CLS(void) //清屏
{
	unsigned char m, n;
	for (m = 0; m < SCREEN_PAGE_NUM; m++)
	{
		for (n = 0; n < SCREEN_COLUMN; n++)
		{
			ScreenBuffer[m][n] = 0;
		}
	}
#if (TRANSFER_METHOD == HW_SPI_DMA) || (TRANSFER_METHOD == HW_IIC_DMA)
	DMA_WriteDat(ScreenBuffer[0], 1024);
#else
	WriteDat(ScreenBuffer[0], 1024);
#endif
}

void OLED_ON(void)
{
	uint8_t buf[] = {0x8d, 0x14, 0xaf};
#if (TRANSFER_METHOD == HW_SPI_DMA) || (TRANSFER_METHOD == HW_IIC_DMA)
	DMA_WriteCmd(buf, sizeof(buf));
#else
	WriteCmd(buf, sizeof(buf));
#endif
}

void OLED_OFF(void)
{
	uint8_t buf[] = {0x8d, 0x10, 0xae};
#if (TRANSFER_METHOD == HW_SPI_DMA) || (TRANSFER_METHOD == HW_IIC_DMA)

	DMA_WriteCmd(buf, sizeof(buf));
#else
	WriteCmd(buf, sizeof(buf));

#endif
}

void OLED_FILL(unsigned char BMP[])
{
#if (TRANSFER_METHOD == HW_SPI_DMA) || (TRANSFER_METHOD == HW_IIC_DMA)

	// * DMA
	DMA_WriteDat(BMP, 1024);

#else
	WriteDat(BMP, 1024);
#endif
}
