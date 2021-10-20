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

extern unsigned char ScreenBuffer[SCREEN_PAGE_NUM][SCREEN_COLUMN];

// OLED 初始化指令数组
static uint8_t OLED_Init_CMD[] =
	{0xAE, 0x20, 0x10, 0xb0, 0xc8, 0x00, 0x10, 0x40, 0x81, 0xff,
	 0xa1, 0xa6, 0xa8, 0x3F, 0xa4, 0xd3, 0x00, 0xd5, 0xf0, 0xd9,
	 0x22, 0xda, 0x12, 0xdb, 0x20, 0x8d, 0x14, 0xaf};

#if (TRANSFER_METHOD == HW_IIC) //1.硬件IIC

//...

//...

extern DMA_HandleTypeDef hdma_i2c1_tx;
extern I2C_HandleTypeDef hi2c1;

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
#if (USING_DMA)
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

#if USING_DMA

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

	OLED_GPIO_RCC_ENABLE();

	GPIO_InitStructure.Pin = OLED_I2C_SCL_PIN | OLED_I2C_SDA_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP; /* 开漏输出 */
	HAL_GPIO_Init(OLED_GPIO_PORT_I2C, &GPIO_InitStructure);

	IIC_SDA_SET;
	IIC_SCL_SET;
}

// 等待应答
static uint8_t I2C_WaiteAck(void)
{
	uint8_t i;

	IIC_SDA_SET;
	delay_us(1);
	IIC_SCL_SET;
	delay_us(1);

	while (OLED_GPIO_PORT_I2C->IDR & OLED_I2C_SDA_PIN)
	{
		i++;
		if (i > 128)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL_CLS;

	return 0;
}

/**
 * @brief  		I2C_WriteByte，向OLED寄存器地址写一个byte的数据
 * @param[in] 	addr：寄存器地址 data：要写入的数据
 * @retval 		无
 * */
void I2C_WriteByte(uint8_t addr, uint8_t data)
{
	IIC_Start();
	IIC_Send_Byte(OLED_ADDRESS);
	I2C_WaiteAck();
	IIC_Send_Byte(addr); //write data
	I2C_WaiteAck();
	IIC_Send_Byte(data);
	I2C_WaiteAck();
	IIC_Stop();
}

void WriteCmd(unsigned char *cmd, int len) //写命令
{
	for (uint16_t i = 0; i < len; i++)
	{
		I2C_WriteByte(0x00, cmd[i]);
	}
}

void WriteDat(unsigned char *dat, int len) //写数据
{
	for (uint16_t i = 0; i < len; i++)
	{
		I2C_WriteByte(0x40, dat[i]);
	}
}

//以下函数开始为软件IIC驱动
//产生IIC起始信号
void IIC_Start(void)
{
	IIC_SDA_SET;
	IIC_SCL_SET;
	delay_us(4);
	IIC_SDA_CLS;
	delay_us(4);
	IIC_SCL_CLS;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(uint8_t IIC_Byte)
{
	unsigned char i;

	IIC_SCL_CLS;

	for (i = 0; i < 8; i++)
	{
		if (IIC_Byte & 0x80)
			IIC_SDA_SET;
		else
			IIC_SDA_CLS;
		IIC_Byte <<= 1;

		delay_us(2);
		IIC_SCL_SET;
		delay_us(2); //必须有保持SCL脉冲的延时
		IIC_SCL_CLS;
		delay_us(2);
	}
	//原程序这里有一个拉高SDA，根据OLED的要求，此句必须去掉。
	// IIC_SDA_SET;
	// IIC_SCL_SET;
	// delay_us(1);
	// IIC_SCL_CLS;
}

//产生IIC停止信号
void IIC_Stop(void)
{
	IIC_SCL_CLS;
	IIC_SDA_CLS;
	delay_us(4);
	IIC_SCL_SET;
	delay_us(4);
	IIC_SDA_SET;
	delay_us(4);
}

#elif (TRANSFER_METHOD == HW_SPI) //3.硬件SPI

#define OLED_RESET_LOW() HAL_GPIO_WritePin(SPI_RES_GPIOx, SPI_RES_PIN, GPIO_PIN_RESET) //低电平复位
#define OLED_RESET_HIGH() HAL_GPIO_WritePin(SPI_RES_GPIOx, SPI_RES_PIN, GPIO_PIN_SET)

#define OLED_CMD_MODE() HAL_GPIO_WritePin(SPI_DC_GPIOx, SPI_DC_PIN, GPIO_PIN_RESET) //命令模式
#define OLED_DATA_MODE() HAL_GPIO_WritePin(SPI_DC_GPIOx, SPI_DC_PIN, GPIO_PIN_SET)	//数据模式

#define OLED_CS_HIGH() HAL_GPIO_WritePin(SPI_CS_GPIOx, SPI_CS_Pin_x, GPIO_PIN_RESET)
#define OLED_CS_LOW() HAL_GPIO_WritePin(SPI_CS_GPIOx, SPI_CS_Pin_x, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	SPI_RCC_SPIx_EN();
	SPI_RCC_GPIOx_EN();

	//* 软件 片选引脚
	GPIO_InitStructure.Pin = SPI_CS_Pin_x;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(SPI_CS_GPIOx, &GPIO_InitStructure);
	OLED_CS_HIGH();

	// * SPI 引脚
	GPIO_InitStructure.Pin = SPI_HW_ALL_PINS;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	HAL_GPIO_Init(SPI_HW_ALL_GPIOx, &GPIO_InitStructure);

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}

#if USING_DMA
	hdma_spi1_tx.Instance = DMA2_Stream3;
	hdma_spi1_tx.Init.Channel = DMA_CHANNEL_3;
	hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_spi1_tx.Init.Mode = DMA_NORMAL;
	hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK)
	{
		Error_Handler();
	}

	__HAL_LINKDMA(spiHandle, hdmatx, hdma_spi1_tx);

	/* SPI1 interrupt Init */
	HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SPI1_IRQn);
#endif

	// * 复位引脚
	GPIO_InitStructure.Pin = SPI_RES_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(SPI_RES_GPIOx, &GPIO_InitStructure);

	// * DC 引脚
	GPIO_InitStructure.Pin = SPI_DC_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(SPI_DC_GPIOx, &GPIO_InitStructure);
	OLED_DATA_MODE();

	OLED_RESET_HIGH();
	delay_ms(100);
	OLED_RESET_LOW();
	delay_ms(100);
	OLED_RESET_HIGH();
}

/**
 * @brief 		SPI 写字节
 * 
 * @param dat 	写入数据地址
 * @param len 	写入数据长度
 */
void SPI_WriterByte(unsigned char *dat, int len)
{
	HAL_SPI_Transmit(&hspi1, dat, len, 1000);
}

/**
 * @brief 		写数据
 * 
 * @param dat 	命令地址
 * @param len 	命令长度
 */
void WriteCmd(unsigned char *cmd, int len)
{
	OLED_CMD_MODE();
	OLED_CS_LOW();
	SPI_WriterByte(cmd, len);
	OLED_CS_HIGH();
	OLED_DATA_MODE();
}

/**
 * @brief 		写数据
 * 
 * @param dat 	数据地址
 * @param len 	数据长度
 */
void WriteDat(unsigned char *dat, int len)
{
	OLED_DATA_MODE();
	OLED_CS_LOW();
	SPI_WriterByte(dat, len);
	OLED_CS_HIGH();
	OLED_DATA_MODE();
}

#elif (TRANSFER_METHOD == SW_SPI) //4.软件SPI

/**
 * @brief 引脚初始化
 * 
 */
void SPI_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	OLED_GPIO_RCC_ENABLE(); //使能B端口时钟
	GPIO_InitStructure.Pin = OLED_SPI_CLK_PIN | OLED_SPI_DIN_PIN | OLED_SPI_RES_PIN | OLED_SPI_DC_PIN | OLED_SPI_CS_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		  //推挽输出
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //速度50MHz
	HAL_GPIO_Init(OLED_GPIO_PORT_SPI, &GPIO_InitStructure);
	HAL_GPIO_WritePin(OLED_GPIO_PORT_SPI, OLED_SPI_CLK_PIN | OLED_SPI_DIN_PIN | OLED_SPI_DC_PIN, GPIO_PIN_SET);

	OLED_RST_SET;
	delay_ms(100);
	OLED_RST_CLS;
	delay_ms(200);
	OLED_RST_SET;
}

/**
 * @brief 		写字节
 * 
 * @param dat 	写入的字节
 * @param cmd 	写命令|写数据
 */
void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
	uint8_t i;
	if (cmd)
		OLED_DC_SET;
	else
		OLED_DC_CLS;
	OLED_CS_CLS;
	for (i = 0; i < 8; i++)
	{
		OLED_SCLK_CLS;
		if (dat & 0x80)
			OLED_SDIN_SET;
		else
			OLED_SDIN_CLS;
		OLED_SCLK_SET;
		dat <<= 1;
	}
	OLED_CS_SET;
	OLED_DC_SET;
}

/**
 * @brief 		写命令
 * 
 * @param cmd 	命令
 * @param len 
 */
void WriteCmd(unsigned char *cmd, int len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		OLED_WR_Byte(cmd[i], 0);
	}
}

//写数据
void WriteDat(unsigned char *dat, int len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		OLED_WR_Byte(dat[i], 1);
	}
}

#endif

void OLED_Init(void)
{
	int length = sizeof(OLED_Init_CMD) / sizeof(OLED_Init_CMD[0]);
#if USING_DMA
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
#if USING_DMA
	DMA_WriteDat(ScreenBuffer[0], 1024);
#else
	WriteDat(ScreenBuffer[0], 1024);
#endif
}

void OLED_ON(void)
{
	uint8_t buf[] = {0x8d, 0x14, 0xaf};
#if USING_DMA
	DMA_WriteCmd(buf, sizeof(buf));
#else
	WriteCmd(buf, sizeof(buf));
#endif
}

void OLED_OFF(void)
{
	uint8_t buf[] = {0x8d, 0x10, 0xae};
#if USING_DMA
	DMA_WriteCmd(buf, sizeof(buf));
#else
	WriteCmd(buf, sizeof(buf));
#endif
}

void OLED_FILL(unsigned char BMP[])
{
#if USING_DMA
	// * DMA
	DMA_WriteDat(BMP, 1024);

#else
	WriteDat(BMP, 1024);
#endif
}

#if USING_DMA
unsigned char DMA_Finish = 0; // DMA 状态

void lockedBuffer(void)
{
	DMA_Finish = 1;
}

void unlockBuffer(void)
{
	DMA_Finish = 0;
}

unsigned char getBufferState(void)
{
	return DMA_Finish;
}
void DMATransmitCallback(void)
{
	if (IIC_DMA->HISR & IIC_DMA_TCIF)
	{
		unlockBuffer();
	}
}
#endif
