# OLED 绘图模板

## OLED 驱动
预计支持的驱动
* [x]   HW_IIC              硬件 IIC 驱动
* [x]   HW_IIC_DMA          硬件 IIC+DMA 驱动
* [x]   SW_IIC              软件 IIC 驱动
* [ ]   HW_SPI              硬件 SPI 驱动
* [ ]   HW_SPI_DMA          硬件 SPI+DMA 驱动
* [ ]   SW_SPI              软件 SPI 驱动

## 配置
	该库只是一个绘图图形库, 只能适用于 `stm32` 系列的 `12864oled` 屏, 驱动芯片为 `ssd1306`, 若需被其他平台使用, 请自行移植
模式选择在 `oled_config.h` 中配置, 如果想自行添加新的驱动模式,可在 `oled_base.h` 中添加
如果不使用 `STM32CubeMX` 初始化 OLED 驱动引脚,请在 `oled_driver.c` 中的 `xxx_Config` 中配置自行配置 OLED 底层驱动
如果不使用 `STM` 芯片, 也只需修改 `oled_driver.c` 里的 各个基础函数即可

## 移植
通信方式只需要在 `oled_config` 修改 `TRANSFER_METHOD` 的宏即可
### IIC
IIC_DMA移植时需要在 DMA 中断中放入回调函数 `DMATransmitCallback();`
`FrameRateUpdateScreen()` 每次会清空整个屏幕, 固定显示某些数据则需要反复写入数组中,这个函数DMA模式下不稳定, 不建议使用
软件 IIC 最快速率为 150k 显示动态图会有明显延时
硬件 IIC 最快速率为 400k 这个速率需要配合DMA实现, 但 IIC 的 OLED 驱动芯片已经难以承受
### SPI


## 示例:
手动刷新
```c
int main(void)
{
	...
	//  /* Initialize all configured peripherals */
	...
  	MX_I2C1_Init();
	...

  	InitGraph();
	UpdateScreen();	// * 手动刷新
  	OledPrintf(" Chinese Flash: %x\n", CN16CHAR[0].Index[0]<<16 | CN16CHAR[0].Index[1]<<8 | CN16CHAR[0].Index[2]);
  	UpdateScreen();
	HAL_Delay(2000);
	OLED_CLS();
  
  	ShowChinese(0, 0, "一只程序羊");
	ShowChinese(1, 4, "求打赏");
	SetFontSize(0);
	DrawString(6, 6 * 8, "blacksheep");
  	HAL_Delay(5000);
	...

  	while(1)
	{
		
	}
	...

	...
	return 0;
}
```
自动刷新
```c
// stm32f4xx_it.c
#include "oled_basic.h"
...
void DMA1_Stream6_IRQHandler(void)
{
	/* USER CODE BEGIN DMA1_Stream6_IRQn 0 */
  	DMATransmitCallback();
  	/* USER CODE END DMA1_Stream6_IRQn 0 */
  	HAL_DMA_IRQHandler(&hdma_i2c1_tx);
  	/* USER CODE BEGIN DMA1_Stream6_IRQn 1 */

  	/* USER CODE END DMA1_Stream6_IRQn 1 */
}
...
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  OledTimeMsFunc();
  /* USER CODE END SysTick_IRQn 1 */
}
...
// main 
int main()
{
	...
	InitGraph();
	...
	while(1)
	{
		...
		DrawString(6, 6, "blacksheep");
    	FrameRateUpdateScreen(60);
		...
	}

	return 0;
}
``` 

