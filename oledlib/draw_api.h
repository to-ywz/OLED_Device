/**
 * @file draw_api.h 
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief 	OLED 绘图库
 * @version 0.1
 * @date 2021-08-28
 * 
 * @note	
 * 			本库函数注释采用 Doxygen 格式, 
 * 		如果安装了 vscode 建议在 vscode 中安装 Better Comments
 * 		便于分别高亮注释高亮
 * 
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// * 坐标分布
/**
 * 		x (0~127)
 * ----------------------
 * |					|
 * |					|
 * |y(0~63)				|
 * |					|
 * |					|
 * ----------------------
 * */

#ifndef _OLED_DRAW_API_H
#define _OLED_DRAW_API_H
#include "oled_config.h"
#include "oled_driver.h"
#include "oled_draw.h"
#include "oled_basic.h"
#include "oled_color.h"
#include "oled_buffer.h"
#include "oled_font.h"
#include "oled_bmp.h"
#include "oled_debug.h"

//* API =================================================================
// * OLED 初始化配置接口, 可自行形成 OLED_GraphInit()
/**
 * @brief 	初始化图形库
 * 
 */
void InitGraph(void);

/**
 * @brief 	将屏幕数据更新到设备上
 * 
 */
void UpdateScreen(void);

/**
 * @brief 	清屏
 * 
 */
void ClearScreen(void);

/**
 * @brief 	为图形提供时基
 * 
 * @note  
 * !		请将此函数放入1ms中断里，
 */
void OledTimeMsFunc(void);

/**
 * @brief 	统一 OLED 延时接口
 * 
 * @param[in] ms 	x ms
 * 
 * @note	
 * 		可以注释该函数, 使用 HAL_Delay 函数来代替
 */
void DelayMs(uint16_t ms);

/**
 * @brief 设置 画笔颜色
 * 
 * @param[in] value pix_black:	黑色
 * 					pix_white:	白色
 */
void SetDrawColor(Type_color value);

/**
 * @brief 	获取当画笔颜色
 * 
 * @retval 	pix_black:	黑色
 * 			pix_white:	白色
 */
Type_color GetDrawColor(void);

/**
 * @brief	设置背景填充色
 * 
 * @param[in] value pix_black:	黑色
 * 					pix_white:	白色
 */
void SetFillcolor(Type_color value);

/**
 * @brief 	获取当前背景填充色
 * 
 * @retval 	pix_black:	黑色
 * 			pix_white:	白色 
 */
Type_color GetFillColor(void);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// * 一下为 OLED 绘图函数库, 可直接调用
// * 位于 oled_draw.h
// * 参考 中景园电子 0.96 寸 OLED 显示屏代码进行修改

/**
 * @brief 	绘制一个点
 * 
 * @param[in] x 横坐标
 * @param[in] y 纵坐标
 */
void DrawPixel(int x, int y);

/**
 * @brief 	获取一个像素点的颜色
 * 
 * @param[in] x 横坐标
 * @param[in] y 纵坐标
 * 
 * @retval 	pix_black:	黑色
 * 			pix_white:	白色 
 */
Type_color GetPixel(int x, int y);

// * 图像绘制================================================================
// * 线绘制
/**
 * @brief 		绘制一条任意直线
 * 
 * @param[in] x1 起始点横坐标
 * @param[in] y1 起始点纵坐标
 * @param[in] x2 终点横坐标
 * @param[in] y2 终点纵坐标
 */
void DrawLine(int x1, int y1, int x2, int y2);

/**
 * @brief 	快速绘制一条水平线 w为长度 ------>
 * 
 * @param[in] x 起始横坐标
 * @param[in] y 起始纵坐标
 * @param[in] w 线宽
 * 
 * @example		DrawFastHLine(0, 63, 128)
 */
void DrawFastHLine(int x, int y, unsigned char w);

/**
 * @brief 	快速绘制一条垂直线 
 * 
 * @param[in] x 起始横坐标
 * @param[in] y 其实纵坐标
 * @param[in] h 高度
 * 
 * @example		 DrawFastVLine(127, 0, 64)
 */
void DrawFastVLine(int x, int y, unsigned char h);

/**
 * @brief 	连续画线 折线 num 为折点个数
 * 
 * @param[in] points 	坐标数组
 * @param[in] num 		坐标数量
 * 
 * @example	  TypeXY Point[5] = {{0, 63}, {8, 32}, {16, 63}, {64, 32}, {127, 63}};	
 * 			  DrawPolyLineTo(Point, sizeof(Point) / sizeof(TypeXY));	
 */
void DrawPolyLineTo(const TypeXY *points, int num);

// * 绘制矩形
/**
 * @brief 		使用对角点绘制矩形
 * 
 * @param[in] left 		左上角 x
 * @param[in] top 		左上角 y
 * @param[in] right		右下角 x
 * @param[in] bottom 	右下角 y
 * 
 * @example	  DrawRect1(0, 0, 127, 63);
 */
void DrawRect1(int left, int top, int right, int bottom);

/**
 * @brief 			使用对角点填充矩形
 * 
 * @param[in] left 		左上角 x
 * @param[in] top 		左上角 y
 * @param[in] right		右下角 x
 * @param[in] bottom 	右下角 y
 * 
 * @example		DrawFillRect1(0, 0, 127, 63);
 */
void DrawFillRect1(int left, int top, int right, int bottom);

/**
 * @brief 			使用顶点 宽高绘制矩形
 * 
 * @param[in] left 		左上角 x
 * @param[in] top 		左上角 y
 * @param[in] width 	正方形的宽
 * @param[in] height 	正方形的高
 * 
 * @example			DrawRect2(0, 0, 127, 63);
 */
void DrawRect2(int left, int top, int width, int height);

/**
 * @brief 			使用顶点 宽高填充矩形
 * 
 * @param[in] left 		左上角 x
 * @param[in] top 		左上角 y
 * @param[in] width 	正方形的宽
 * @param[in] height 	正方形的高
 * 
 * @example			DrawFillRect2(0, 0, 127, 63);
 */
void DrawFillRect2(int left, int top, int width, int height);

// * 圆形绘制
/**
 * @brief 				绘制一个圆
 * 
 * @param[in] usX_Center 	中心点坐标 x
 * @param[in] usY_Center 	中心点坐标 x
 * @param[in] usRadius 		半径
 * 
 * @example				DrawCircle(16, 8, 8);
 */
void DrawCircle(int usX_Center, int usY_Center, int usRadius);

/**
 * @brief 				绘制一个圆
 * 
 * @param[in] usX_Center 	中心点坐标 x
 * @param[in] usY_Center 	中心点坐标 x
 * @param[in] usRadius 		半径
 * 
 * @example				DrawFillCircle(109, 8, 8);
 */
void DrawFillCircle(int usX_Center, int usY_Center, int usRadius);

/**
 * @brief 				4份之一圆要画哪一份或哪几份
 * 
 * @param[in] x0 			中心坐标 x
 * @param[in] y0 			中心坐标 y
 * @param[in] r 			半径
 * @param[in] cornername 	0xxx(0~15)
 * 			0		0		0		0
 * 			右上	右下	左下	左上
 * @example					DrawCircleHelper(63, 31, 10, 0x8);
 * 
 * @note	由于枚举不便于处理, 而且穷举状态过于复杂,所以 通过模仿寄存器 置位
 */
void DrawCircleHelper(int x0, int y0, unsigned char r, unsigned char cornername);

/**
 * @brief               填充2个四分之一圆 和 中间的矩形
 * 
 * @param[in] x0            中心坐标 x   
 * @param[in] y0            中心坐标 y
 * @param[in] r             半径
 * @param[in] cornername    (0x1~0x3)
 * @param[in] delta         中间长度
 * 
 * @example                 DrawFillCircleHelper(63, 31, 10, 0x3, 0);
 * 
 * @note    建议在 绘制圆角矩形之外不要使用, 不要用于圆形绘制, 会出现十字空隙
 */
void DrawFillCircleHelper(int x0, int y0, unsigned char r, unsigned char cornername, int delta);

// * 弧线绘制(存在bug)
//功能:
/**
 * @brief               绘制一个圆弧
 * 
 * @param[in] x             中心坐标 x
 * @param[in] y             中心坐标 y
 * @param[in] r             半径
 * @param[in] angle_start   开始度数
 * @param[in] angle_end     终止度数
 * 
 * @example             DrawArc(63, 31, 10, 0, 360);
 * 
 * @note        由于 实现算法存在问题,所以 不推荐使用
 */
void DrawArc(int x, int y, unsigned char r, int angle_start, int angle_end);

/**
 * @brief               填充一个圆弧
 * 
 * @param[in] x             中心坐标 x
 * @param[in] y             中心坐标 y
 * @param[in] r             半径
 * @param[in] angle_start   开始度数
 * @param[in] angle_end     终止度数
 * 
 * @example             None
 * 
 * !@note                未实现
 */
// void DrawFillArc(int x, int y, unsigned char r, int angle_start, int angle_end);

// * 圆角矩形
/**
 * @brief           绘制一个圆角矩形
 * 
 * @param[in] x     横坐标         
 * @param[in] y     纵坐标
 * @param[in] w     长
 * @param[in] h     宽
 * @param[in] r     弧度
 * 
 * @example         DrawRoundRect(2, 2, 125, 62, 15);
 */
void DrawRoundRect(int x, int y, unsigned char w, unsigned char h, unsigned char r);

/**
 * @brief           绘制一个填充圆角矩形
 * 
 * @param[in] x     横坐标         
 * @param[in] y     纵坐标
 * @param[in] w     长
 * @param[in] h     宽
 * @param[in] r     弧度
 * 
 * @example         
 */
void DrawfillRoundRect(int x, int y, unsigned char w, unsigned char h, unsigned char r);

//////////////////////////////////////////////////////////////////////////////////
// * 椭圆
/**
 * @brief           绘制一个椭圆
 * 
 * @param[in] x0    横坐标
 * @param[in] y0    纵坐标
 * @param[in] a     长轴长
 * @param[in] b     短轴长
 * 
 * @example         DrawEllipse(32, 32, 15, 10);
 */
void DrawEllipse(int x0, int y0, int a, int b);

/**
 * @brief           绘制一个填充椭圆
 * 
 * @param[in] x0    横坐标
 * @param[in] y0    纵坐标
 * @param[in] a     长轴长
 * @param[in] b     短轴长
 * 
 * @example         DrawFillEllipse(32, 32, 15, 10);
 */
void DrawFillEllipse(int x0, int y0, int a, int b);

/**
 * @brief           绘制一个矩形内切椭圆
 * 
 * @param x0 		左上角
 * @param y0 
 * @param x1 		右下角
 * @param y1 
 * 
 * @example         DrawEllipseRect(10, 10, 70, 40);
 */
void DrawEllipseRect(int x0, int y0, int x1, int y1);

////////////////////////////////////////////////////////////////////////////
// * 三角形
/**
 * @brief           三点绘制三角形
 * 
 * @param[in] xi     第i个点 x   
 * @param[in] yi     第i个点 y
 * 
 * @example         DrawTriangle(10, 10, 70, 40, 90, 40);
 */
void DrawTriangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);

/**
 * @brief           三点绘制填充三角形
 * 
 * @param[in] xi     第i个点 x   
 * @param[in] yi     第i个点 y
 * 
 * @example         DrawFillTriangle(10, 10, 70, 40, 90, 40);
 */
void DrawFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2);

/**
 * @brief               绘制一张图片 bitmap为图片数据
 *                      （可通过取模获取，PCtoLCD2002参数请选择[阴码 列行式 逆向]）
 * 
 * @param[in] x         起始坐标 x
 * @param[in] y         起始坐标 y
 * @param[in] bitmap    图像
 * @param[in] w         长
 * @param[in] h         宽
 */
void DrawBitmap(int x, int y, const unsigned char *bitmap, unsigned char w, unsigned char h);

/**
 * @brief               绘制一张 XBM 图像
 * 
 * @param[in] x         起始坐标 x
 * @param[in] y         起始坐标 y
 * @param[in] bitmap    图像
 * @param[in] w         长
 * @param[in] h         宽
 * 
 * @note XBM：1、是一种标准的图片格式，可用ACDsee等看图软件打开，用到单片机编程中，可使图片不用取模软件取模就可得出图片代码，
 * 在[https://onlineconvertfree.com/zh/convert/xbm/]可转换单色图片到XBM格式，之后用记事本打开即可得到图片代码
 *            2、如果要用取模软件转换，PCtoLCD2002参数请选择[阴码 逐行式 逆向]
 *            3、因为与SSD1306内部数据存储方式不一样，涉及到数据转换问题，速度较[DrawBitmap]慢，但几乎可以忽略不计。
 */
void DrawXBitmap(int x, int y, const unsigned char *bitmap, unsigned char w, unsigned char h);

////////////////////////////////////////////////////////////////////////////////////////////////////////
// * 字符串显示配置
/**
 * @brief           设置字体大小
 * 
 * @param[in] value   0 1 2 3
 *                  一般建议使用大小为 0
 */
void SetFontSize(unsigned char value);

/**
 * @brief           获取字体大小
 * 
 * @retval          字体大小
 */
unsigned char GetFontSize(void);

/**
 * @brief           设置填充字符时的背景颜色
 * 
 * @param[in] value TEXT_BK_NULL | TEXT_BK_NOT_NULL
 *                  对于 oled 只有 有颜色或者没有颜色, 用于 LCD 拓展
 */
void SetTextBkMode(Type_textbk value);

/**
 * @brief           获取填充字符时的背景颜色
 * 
 * @retval          TEXT_BK_NULL | TEXT_BK_NOT_NULL
 */
Type_textbk GetTextBkMode(void);

/**
 * @brief           绘制一个字符
 * 
 * @param[in] x y   开始坐标     
 * @param[in] c     字符
 * @note            当size=0时 x为0-7行 y为0-127列
 */
void DrawChar(int x, int y, unsigned char c);

/**
 * @brief           绘制字符串
 * 
 * @param[in] x y   开始坐标     
 * @param[in] str   字符
 * @note            当size=0时 x为0-7行 y为0-127列
 */
void DrawString(int x, int y, char *str);

/**
 * @brief           绘制数字
 * 
 * @param[in] x y   开始坐标     
 * @param[in] num   数字
 * @param[in] len   长度
 * @note            当size=0时 x为0-7行 y为0-127列
 */
void DrawNum(unsigned char x, unsigned char y, unsigned int num, unsigned char len);

/**
 * @brief           显示汉字
 * 
 * @param[in] x y   开始坐标     
 * @param[in] cn    汉字
 * @note            当size=0时 x为0-7行 y为0-127列
 */
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t *cn);

/**
 * @brief 
 * 
 * @param[in] x, y	显示字符需要所在的坐标对 	
 * @param[in] str 	中文
 * 
 * @example			ShowChinese(0, 0, "一只程序羊");
 * 
 * @note			汉字需要使用PCtoLCD2002生成, 并添加到 oled_font.c文件中
 * !				字模大小为 16,所以
 * 					x 值为 0~3,
 * 					y 值为 0~7
 */
void ShowChinese(int x, int y, const char *str);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// * 以下函数为oled.basic中直接供用户调用的函数
// * 一般来说,除非绘制 OLED 菜单否则基本不会使用
// * 非常不建议直接调用

/**
 * @brief               可用于填充一个封闭图形
 * 
 * @param[in] x,y       图像坐标
 * @param[in] oldcolor  原本的颜色
 * @param[in] newcolor  要染成的颜色
 * @note                在填充圆弧中非常适合使用
 */
void FloodFill2(unsigned char x, unsigned char y, int oldcolor, int newcolor);

/**
 * @brief               移动绘制坐标
 * 
 * @param[in] x,y       将"光标"移动到下一个要绘制的地方
 */
void MoveTo(int x, int y);

/**
 * @brief               线性绘制,并将绘制坐标移动到下一个坐标
 * 
 * @param[in] x,y       将绘制横线的"光标"移动到下一个要绘制的地方
 * @note                在绘制折线中被调用
 */
void LineTo(int x, int y);

/**
 * @brief       获取当前点绘制点的y坐标
 * 
 */

int GetY(void);
/**
 * @brief       获取当前点绘制点的x坐标
 * 
 */

int GetX(void);
/**
 * @brief       获取当前点绘制点的 xy 坐标对
 * 
 */
TypeXY GetXY(void);


/** 
 * @brief               设置旋转中心点
 * 
 * @param[in] x0,y0         选装中心坐标
 */

void SetRotateCenter(int x0, int y0);

/**
 * @brief               设置旋转中心点
 * 
 * @param[in] angle         角度
 */
void SetAngle(float angle);

/**
 * @brief               设置旋转方向
 * 
 * @param[in] direction     0 顺 1逆
 */
void SetAnggleDir(int direction);

/**
 * @brief               设置角度、旋转方向、旋转中心
 * 
 * @param[in] x y       旋转中心
 * @param[in] angle     旋转角度
 * @param[in] direct    旋转方向
 */
void SetRotateValue(int x, int y, float angle, int direct);

/**
 * @brief               将一个坐标旋转一定角度
 * 
 * @param[in] x,y       要被旋转的坐标
 * @retval              旋转后的 坐标对
 */
TypeXY GetRotateXY(int x, int y);

/**
 * @brief  选择屏幕数据缓冲
 * 
 */
void SetScreenBuffer(void);

/**
 * @brief 选择设置新的缓冲作为屏幕数据
 * 
 */
void SetTempBuffer(void);

/**
 * @brief 获取当前的缓冲buffer的像素 返回缓冲系数SCREEN_BUFFER或TEMP_BUFFER
 * 
 * @retval 当前缓冲区中的buffer的像素
 */
unsigned char GetSelectedBuffer(void);

/**
 * @brief 清除掉默认的屏幕缓冲数据  
 * 
 * @param val 在 OLED为 0
 */
void ClearScreenBuffer(unsigned char val);

/**
 * @brief 清除掉临时的屏幕缓冲数据
 * 
 */
void ClearTempBuffer(void);

/**
 * @brief 			对临时缓冲进行一些操作
 * 
 * @param func		执行的功能可选择的参数如下
 * 	TEMPBUFF_COPY_TO_SCREEN, 		 将temp缓冲复制到屏幕缓冲
 * 	TEMPBUFF_CLEAN,					 清楚掉temp缓冲数据
 * 	TEMPBUFF_COVER_L,				 将temp缓冲的数据取反再覆盖掉屏幕上的数据
 * 	TEMPBUFF_COVER_H				 将temp缓冲的数据覆盖掉屏幕上的数据 
 */
void TempBufferFunc(int func);

/**
 * @brief 固定帧刷新
 * 此函数放在while循环中 符合条件时刷新屏幕
 * @param value 	帧数
 * @retval 	是否到达刷新时间
 * @note	OledTimeMs在OledTimeMsFunc()中被1ms中断持续调用 减到0位置
 */
unsigned char FrameRateUpdateScreen(int value);

//////////////////////////////////////////////////////////////////////////////////////
// * 调试使用
/**
 * @brief       用以打印调试bug，功能和printf一致，
 *              不用使用ClearScreen方法也可打印出数据
 * 
 * @param str   格式化字符串
 * @param ... 
 * @retval 
 */
int OLED_printf(const char *str, ...);

#endif
