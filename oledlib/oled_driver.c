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

#if USING_DMA

typedef enum DMA_LOCK
{
	UNLOCK = 0, // 解锁
	LOCKED = 1	// 锁
} DMA_LOACK_T;

static DMA_LOACK_T DMA_State;

void lockedBuffer(void)
{
	DMA_State = LOCKED;
}

void unlockBuffer(void)
{
	DMA_State = UNLOCK;
}

unsigned char getBufferState(void)
{
	return DMA_State;
}
#endif

#if (TRANSFER_METHOD == HW_IIC) // 1.硬件IIC

//...

//...
#if USING_DMA
extern DMA_HandleTypeDef hdma_i2c1_tx;
#endif
extern I2C_HandleTypeDef hi2c1;

// I2C_Configuration，初始化硬件IIC引脚
//  ! 建议在 STM32CubeMX中直接配置
void I2C_Configuration(void)
{
	// 维持接口统一
}

#if USING_DMA

/**
 * @brief DMA 发送结束回调函数
 * 			防止数据覆盖
 */
void DMATransmitCallback(void)
{
	if (IIC_DMA_SRREG & IIC_DMA_TCIF)
	{
		unlockBuffer();
	}
}

#endif
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
	HAL_I2C_Mem_Write_DMA(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteData_Addr, I2C_MEMADD_SIZE_8BIT, dat, len);
#else
	HAL_I2C_Mem_Write(HWI2Cx_12864, OLED_ADDRESS, OLED_WriteData_Addr, I2C_MEMADD_SIZE_8BIT, dat, len, 1000);
#endif
}


#elif (TRANSFER_METHOD == SW_IIC) // 2.软件IIC

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
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD; /* 开漏输出 */
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
	IIC_Send_Byte(addr); // write data
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

// IIC发送一个字节
//返回从机有无应答
// 1，有应答
// 0，无应答
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

#elif (TRANSFER_METHOD == HW_SPI) // 3.硬件SPI

#define OLED_RESET_LOW() CLEAR_BIT(SPI_RES_GPIOx->ODR, SPI_RES_PIN) //低电平复位
#define OLED_RESET_HIGH() SET_BIT(SPI_RES_GPIOx->ODR, SPI_RES_PIN)

#define OLED_CMD_MODE() CLEAR_BIT(SPI_DC_GPIOx->ODR, SPI_DC_PIN) //命令模式
#define OLED_DATA_MODE() SET_BIT(SPI_DC_GPIOx->ODR, SPI_DC_PIN)	 //数据模式

#define OLED_CS_HIGH() CLEAR_BIT(SPI_CS_GPIOx->ODR, SPI_CS_Pin_x) // 片选 可直接接地放弃这个引脚
#define OLED_CS_LOW() SET_BIT(SPI_CS_GPIOx->ODR, SPI_CS_Pin_x)

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

// I2C_Configuration，初始化硬件IIC引脚
//  ! 建议在 STM32CubeMX中直接配置
void SPI_Configuration(void)
{ // 保留接口一致
	// 必须复位
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
#if USING_DMA
/**
 * @brief DMA 发送结束回调函数
 * 			防止数据覆盖
 */
void DMATransmitCallback(void)
{
	if (SPI_DMA_SRREG & SPI_DMA_TCIF)
	{
		unlockBuffer();
	}
}
#endif
#elif (TRANSFER_METHOD == SW_SPI) // 4.软件SPI

#define OLED_SCLK_CLS CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CLK_PIN) // CLK
#define OLED_SCLK_SET SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CLK_PIN)

#define OLED_SDIN_CLS CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_DIN_PIN) // DIN
#define OLED_SDIN_SET SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_DIN_PIN)

#define OLED_RST_CLS CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_RES_PIN) // RES
#define OLED_RST_SET SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_RES_PIN)

#define OLED_DC_CLS CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_DC_PIN) // DC
#define OLED_DC_SET SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_DC_PIN)

#define OLED_CS_CLS CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CS_PIN) // CS
#define OLED_CS_SET SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CS_PIN)

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

	// 必须复位
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
	WriteCmd(OLED_Init_CMD, length);
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
	WriteDat(ScreenBuffer[0], 1024);
}

void OLED_ON(void)
{
	uint8_t buf[] = {0x8d, 0x14, 0xaf};
	WriteCmd(buf, sizeof(buf));
}

void OLED_OFF(void)
{
	uint8_t buf[] = {0x8d, 0x10, 0xae};
	WriteCmd(buf, sizeof(buf));
}

void OLED_FILL(unsigned char BMP[])
{
	WriteDat(BMP, 1024);
}


