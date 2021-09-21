#ifndef __OLED_DRIVER_H
#define __OLED_DRIVER_H
#include "stm32f4xx.h"
#include "oled_config.h"
#include "oled_basic.h"


#if (TRANSFER_METHOD == HW_IIC) 
#define OLED_WriteData_Addr	0x40
#define OLED_WriteCom_Addr	0x00	

	#define OLED_ADDRESS  0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78
	#if (USE_HW_IIC == IIC_1)
		/*STM32F4芯片的硬件I2C: PB6 -- SCL; PB7 -- SDA */
		#define IIC_I2C_ENABLE()     		__HAL_RCC_I2C1_CLK_ENABLE();
		#define IIC_GPIO_ENABLE()			__HAL_RCC_GPIOB_CLK_ENABLE();
		#define IIC_GPIOX                   GPIOB
		#define IIC_SCL_PIN_X               GPIO_PIN_6
		#define IIC_SDA_PIN_X               GPIO_PIN_7
		#define I2CX                        I2C1
		#define HWI2Cx_12864 				&hi2c1
		
		#if USING_DMA	
			#define IIC_DMA 				DMA1
			#define IIC_DMA_TCIF			DMA_HISR_TCIF6
		#endif

	#elif (USE_HW_IIC == IIC_2)
		#define IIC_I2C_ENABLE()     		__HAL_RCC_I2C2_CLK_ENABLE();
		#define IIC_GPIO_ENABLE()     		__HAL_RCC_GPIOB_CLK_ENABLE();
		#define IIC_GPIOX                   GPIOB
		#define IIC_SCL_Pin_X               GPIO_PIN_10
		#define IIC_SDA_Pin_X               GPIO_PIN_11
		#define I2CX                        I2C2
		#define HWI2Cx_12864 				&hi2c2
	#endif
	void I2C_Configuration(void);
	void I2C_WriteByte(uint8_t addr, uint8_t data);
	
#elif (TRANSFER_METHOD == SW_IIC)//软件I2C依赖主频，在F103（72M）测试下通过，其余请自行测试

	#define OLED_ADDRESS  				0x78 	//通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78
	#define OLED_GPIO_PORT_I2C			GPIOA							/* GPIO端口 */
	#define OLED_GPIO_RCC_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE();	/* GPIO端口时钟 */
	#define OLED_I2C_SCL_PIN			GPIO_PIN_4						/* 连接到SCL时钟线的GPIO */
	#define OLED_I2C_SDA_PIN			GPIO_PIN_6						/* 连接到SDA数据线的GPIO */
	
	#define IIC_SCL_SET             	SET_BIT(OLED_GPIO_PORT_I2C->ODR, OLED_I2C_SCL_PIN) 		//SCL
	#define IIC_SCL_CLS             	CLEAR_BIT(OLED_GPIO_PORT_I2C->ODR, OLED_I2C_SCL_PIN) 	//SCL
	#define IIC_SDA_SET             	SET_BIT(OLED_GPIO_PORT_I2C->ODR, OLED_I2C_SDA_PIN) 		//SDA
	#define IIC_SDA_CLS             	CLEAR_BIT(OLED_GPIO_PORT_I2C->ODR, OLED_I2C_SDA_PIN) 	//SDA
	void I2C_Configuration(void);
	void I2C_WriteByte(uint8_t addr, uint8_t data);


#elif (TRANSFER_METHOD == HW_SPI)

	#if (USE_HW_SPI==SPI_1)
		#define SPIX                      	SPI1
		//使能SPI2时钟
		#define SPI_RCC_SPIx_EN()  	 		__HAL_RCC_SPI1_CLK_ENABLE();
		//使能gpio时钟，使用的GPIO不一样时可定义如下：
		#define SPI_RCC_GPIOx_EN()			__HAL_RCC_GPIOA_CLK_ENABLE();\
											__HAL_RCC_GPIOB_CLK_ENABLE();
		//CS片选（软件片选）
		#define SPI_CS_Pin_x              	GPIO_PIN_10
		#define SPI_CS_GPIOx              	GPIOB
		//SPI2 时钟、mosi、miso引脚
		#define SPI_HW_ALL_PINS           	(GPIO_PIN_5|GPIO_PIN_7)
		#define SPI_HW_ALL_GPIOx          	GPIOA
		//复位引脚
		#define SPI_RES_PIN               	GPIO_PIN_2
		#define SPI_RES_GPIOx             	GPIOB
		//控制引脚
		#define SPI_DC_PIN                	GPIO_PIN_10
		#define SPI_DC_GPIOx              	GPIOB
	#elif (USE_HW_SPI==SPI_2)

		#define SPIX                      	SPI2
		//使能SPI2时钟
		#define SPI_RCC_SPIx_EN()  	 		__HAL_RCC_SPI2_CLK_ENABLE();
		//使能gpio时钟，使用的GPIO不一样时可定义如下：
		#define SPI_RCC_GPIOx_EN()			__HAL_RCC_GPIOA_CLK_ENABLE();\
											__HAL_RCC_GPIOB_CLK_ENABLE();
		//CS片选（软件片选）
		#define SPI_CS_Pin_x              	GPIO_PIN_12
		#define SPI_CS_GPIOx              	GPIOB
		//SPI2 时钟、mosi、miso引脚
		#define SPI_HW_ALL_PINS           	(GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15)
		#define SPI_HW_ALL_GPIOx          	GPIOB
		//复位引脚
		#define SPI_RES_PIN               	GPIO_PIN_10
		#define SPI_RES_GPIOx             	GPIOB
		//控制引脚
		#define SPI_DC_PIN                	GPIO_PIN_111
		#define SPI_DC_GPIOx              	GPIOB

	#endif
	void SPI_Configuration(void);
	// void SPI_WriterByte(unsigned char dat);
	
#elif (TRANSFER_METHOD == SW_SPI)
	
	#define OLED_GPIO_PORT_SPI			GPIOB							/* GPIO端口 */
	#define OLED_GPIO_RCC_ENABLE()		__HAL_RCC_GPIOB_CLK_ENABLE();	/* GPIO端口时钟 */
	#define OLED_SPI_CLK_PIN			GPIO_PIN_0						/* 连接到CLK时钟线的GPIO */
	#define OLED_SPI_DIN_PIN			GPIO_PIN_1						/* 连接到DIN数据线的GPIO */
	#define OLED_SPI_RES_PIN			GPIO_PIN_2						/* 连接到RES复位线的GPIO */
	#define OLED_SPI_DC_PIN				GPIO_PIN_10						/* 连接到CS片选线的GPIO */
	#define OLED_SPI_CS_PIN				GPIO_PIN_12						/* 连接到CS片选线的GPIO */

	#define OLED_SCLK_CLS 				SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CLK_PIN) //CLK
	#define OLED_SCLK_SET 				CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CLK_PIN)

	#define OLED_SDIN_CLS 				HAL_GPIO_WritePin(OLED_GPIO_PORT_SPI, OLED_SPI_DIN_PIN, GPIO_PIN_SET) //DIN
	#define OLED_SDIN_SET 				HAL_GPIO_WritePin(OLED_GPIO_PORT_SPI, OLED_SPI_DIN_PIN, GPIO_PIN_RESET)

	#define OLED_RST_CLS 				SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_RES_PIN) //RES
	#define OLED_RST_SET 				CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_RES_PIN)

	#define OLED_DC_CLS 				SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_DC_PIN) //DC
	#define OLED_DC_SET 				CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_DC_PIN)

	#define OLED_CS_CLS 				SET_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CS_PIN) //CS
	#define OLED_CS_SET 				CLEAR_BIT(OLED_GPIO_PORT_SPI->ODR, OLED_SPI_CS_PIN)

	#define OLED_CMD 0	//写命令
	#define OLED_DATA 1 //写数据
	void SPI_Configuration(void);
	void SPI_WriterByte(unsigned char dat);
	
#endif


void WriteCmd(unsigned char *cmd, int len);
void WriteDat(unsigned char *dat, int len);

void OLED_Init(void);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_FILL(unsigned char BMP[]);


#endif //__OLED_DRIVER_H

