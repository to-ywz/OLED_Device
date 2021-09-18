/**
 * @file oled_buffer.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief 	屏幕数组进行操作
 * @version 0.1
 * @date 2021-09-16
 * 
 * @copyright Copyright (c) 2021
 * 
 * @note
 * 	数组包括屏幕缓冲和临时缓冲
 * 	函数只会目前选择的缓冲进行操作
 *	目前选择的缓冲的选择可通过 SetScreenBuffer 和 SetTempBuffer 来选择
 *	然后就是提供读或写入取缓冲中的8位数据或1位数据的接口
 * 	最后是更新缓冲数据到屏幕
 */

#include "oled_buffer.h"
#include "oled_config.h"
#include "oled_color.h"
#include "string.h"

//定义缓冲 屏幕缓冲区和临时缓冲区
extern unsigned char DMA_Finish;
unsigned char ScreenBuffer[SCREEN_PAGE_NUM][SCREEN_COLUMN] = {0}; //屏幕缓冲
unsigned char TempBuffer[SCREEN_PAGE_NUM][SCREEN_COLUMN] = {0};	  //临时操作缓冲
static _Bool _SelectedBuffer = SCREEN_BUFFER;					  //当前选择的缓冲区

#define BUFFERSIZE sizeof(ScreenBuffer)
extern void UpdateTempBuffer(void);
extern void UpdateScreenBuffer(void);

///////////////////////////////////////////////////////////////////
/**
 * @brief  选择屏幕数据缓冲
 * 
 */
void SetScreenBuffer(void)
{
	_SelectedBuffer = SCREEN_BUFFER;
}

/**
 * @brief 选择设置新的缓冲作为屏幕数据
 * 
 */
void SetTempBuffer(void)
{
	_SelectedBuffer = TEMP_BUFFER;
}
//获取程序目前选择的缓冲区
unsigned char GetSelectedBuffer(void)
{
	return _SelectedBuffer;
}

/**
 * @brief 获取当前的缓冲buffer的像素 返回缓冲系数SCREEN_BUFFER或TEMP_BUFFER
 * 
 * @retval 当前缓冲区中的buffer的像素
 */
void ClearScreenBuffer(unsigned char val)
{
	memset(ScreenBuffer, val, sizeof(ScreenBuffer));
}

/**
 * @brief 清除掉默认的屏幕缓冲数据  
 * 
 * @param val 在 OLED为 0
 */
void ClearTempBuffer(void)
{
	memset(TempBuffer, 0, sizeof(TempBuffer));
}

/**
 * @brief 			对临时缓冲进行一些操作
 * 
 * @param func		执行的功能可选择的参数如下
 * 	TEMPBUFF_COPY_TO_SCREEN, 		 将temp缓冲复制到屏幕缓冲
 * 	TEMPBUFF_CLEAN,					 清楚掉temp缓冲数据
 * 	TEMPBUFF_COVER_L,				 将temp缓冲的数据取反再覆盖掉屏幕上的数据
 * 	TEMPBUFF_COVER_H				 将temp缓冲的数据覆盖掉屏幕上的数据 
 */
void TempBufferFunc(int func)
{
	int i, j;
	switch (func)
	{
	case TEMPBUFF_COPY_TO_SCREEN:
		memcpy(ScreenBuffer, TempBuffer, BUFFERSIZE);
		break;
	case TEMPBUFF_CLEAN:
		ClearTempBuffer();
		break;
	case TEMPBUFF_COVER_H:
		for (i = 0; i < 8; i++)
			for (j = 0; j < 128; j++)
				ScreenBuffer[i][j] |= TempBuffer[i][j];
		break;
	case TEMPBUFF_COVER_L:
		for (i = 0; i < 8; i++)
			for (j = 0; j < 128; j++)
				ScreenBuffer[i][j] &= ~TempBuffer[i][j];
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//读取选择的缓冲区的8位数据
unsigned char ReadByteBuffer(int page, int x)
{
	return _SelectedBuffer ? ScreenBuffer[page][x] : TempBuffer[page][x];
}
//写入读取选择的缓冲区8位数据
void WriteByteBuffer(int page, int x, unsigned char byte)
{
	while (!DMA_Finish)
		;

	if (_SelectedBuffer)
	{
		ScreenBuffer[page][x] = byte;
	}
	else
	{
		TempBuffer[page][x] = byte;
	}
}

//设置当前选择的缓冲区 的 某一个点的亮灭
void SetPointBuffer(int x, int y, int value)
{
	if (x > SCREEN_COLUMN - 1 || y > SCREEN_ROW - 1) //超出范围
		return;
	if (_SelectedBuffer)
	{
		if (value)
			ScreenBuffer[y / SCREEN_PAGE_NUM][x] |= 1 << (y % SCREEN_PAGE_NUM);
		else
			ScreenBuffer[y / SCREEN_PAGE_NUM][x] &= ~(1 << (y % SCREEN_PAGE_NUM));
	}
	else
	{
		if (value)
			TempBuffer[y / SCREEN_PAGE_NUM][x] |= 1 << (y % SCREEN_PAGE_NUM);
		else
			TempBuffer[y / SCREEN_PAGE_NUM][x] &= ~(1 << (y % SCREEN_PAGE_NUM));
	}
}
//获取当前选择的缓冲区 的 某一点的颜色
unsigned char GetPointBuffer(int x, int y)
{
	if (x > SCREEN_COLUMN - 1 || y > SCREEN_ROW - 1) //超出范围
		return 0;
	if (_SelectedBuffer)
		return (ScreenBuffer[y / SCREEN_PAGE_NUM][x] >> (y % SCREEN_PAGE_NUM)) & 0x01;
	else
		return (TempBuffer[y / SCREEN_PAGE_NUM][x] >> (y % SCREEN_PAGE_NUM)) & 0x01;
}
//刷新屏幕显示
void UpdateScreenDisplay(void)
{
	UpdateScreenBuffer();
}
