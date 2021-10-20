#ifndef OLED_CONFIG_H
#define OLED_CONFIG_H

#include "oled_basic.h"

//定义屏幕基本状况
//保持了可拓展性 如果换用其他类型屏
//因注意其他配套c文件中用了这些宏定义的部分

//配置图形库
/*****************************************************************/
//选择屏幕类型
//1.HW_IIC    	硬件IIC 		支持
//2.HW_IIC_DMA 	硬件IIC+DMA		支持
//3.SW_IIC    	软件IIC			支持
//4.HW_SPI		硬件SPI			支持
//5.HW_SPI_DMA	硬件SPI+DMA		支持
//6.SW_SPI		软件SPI			支持
/*****************************************************************/
#define  TRANSFER_METHOD   (HW_SPI) //如果选择了硬件驱动，请在下方选择硬件编号

#if (TRANSFER_METHOD==HW_IIC)
		//IIC_1: PB6 -- SCL; PB7 -- SDA
		//IIC_2: PB10-- SCL; PB11 --SDA
		#define USE_HW_IIC		IIC_1 //IIC2驱动还未测试，现在使用的IIC1
		#define USING_DMA		(1)	// 是否使用DMA	
#elif (TRANSFER_METHOD==HW_SPI)
		//如需更换片选、复位、控制等引脚去oled_driver.h设置
		//SPI_1: PA
		//SPI_2: PB10--RES; PB11--DC; PB12--CS;PB13--D0;PB15--D1;
		#define USE_HW_SPI		SPI_1 //SPI1硬件驱动并未添加
		#define USING_DMA		(1)	// 是否使用DMA
#endif



/*****************************************************************/
//选择屏幕类型
//1.OLED_12864	支持
//2.OLED_12832	不支持 (长屏幕需要不同的算法)
/*****************************************************************/
#define  SCREEN_TYPE   (OLED_12864)

#if (SCREEN_TYPE==OLED_12864)
	#define   	OLED_12864                      	//屏幕类型
	#define 	SCREEN_PAGE_NUM			   	(8)     //总行数 (大行)
	#define   	SCREEN_PAGEDATA_NUM   		(128)   //每大行的列数
	#define  	SCREEN_ROW 					(64)    //总行数
	#define 	SCREEN_COLUMN 				(128)   //总列数
	#if 0
		#define 	SCREEN_PHY_X            (21.744)
		#define 	SCREEN_PHY_Y			(10.864) 
	#endif
#endif


#endif //OLED_CONFIG_H



