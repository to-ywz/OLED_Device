# OLED 绘图模板

## OLED 驱动
预计支持的驱动
* [x]   HW_IIC              硬件 IIC 驱动
* [x]   HW_IIC_DMA          硬件 IIC+DMA 驱动
* [ ]   SW_IIC              软件 IIC 驱动
* [ ]   HW_SPI              硬件 SPI 驱动
* [ ]   HW_SPI_DMA          硬件 SPI+DMA 驱动
* [ ]   SW_SPI              软件 SPI 驱动

## 配置
	该库只是一个绘图图形库, 只能适用于 `stm32` 系列的 `12864oled` 屏, 驱动芯片为 `ssd1306`, 若需被其他平台使用, 请自行移植
模式选择在 `oled_config.h` 中配置, 如果想自行添加新的驱动模式,可在 `oled_base.h` 中添加
如果不使用 `STM32CubeMX` 初始化 OLED 驱动引脚,请在 `oled_driver.c` 中的 `xxx_Config` 中配置自行配置 OLED 底层驱动
如果不使用 `STM` 芯片, 也只需修改 `oled_driver.c` 里的 各个基础函数即可

## 移植
IIC_DMA移植时需要在 DMA 中断中放入回调函数 `DMATransmitCallback();`
`FrameRateUpdateScreen()` 每次会清空整个屏幕, 固定显示某些数据则需要反复写入数组中




## 文件结构

