/**
 * @file oled_draw.c
 * @author BlackSheep (blacksheep.208h@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-1
 * 
 * @copyright Copyright (c) 2021
 * 
 * @note
 * 		此c文件用于画图和字符操作 
 * 		由五部分组成
 * 		1. 打点
 * 		2. 线 折线
 * 		3. 填充 矩形 三角形 圆 椭圆 圆角矩形
 * 		4. 图片 字符 字符串
 * 		5. 汉字
 */

#include "oled_draw.h"
#include "oled_buffer.h"
#include "oled_config.h"
#include "stdlib.h"
#include "math.h"
#include "oled_bmp.h"
#include "oled_font.h"
#include "string.h"

// #define CN_ENCODE_SIZE 3 // utf-8 为 3 字;	GBK|GB2312 为 2 字节

//画图光标
static int _pointx = 0;
static int _pointy = 0;

////////////////////////////////////////////////////////////////////
//以下4个函数是对当前光标的设置 供以下绘制函数调用 用户不直接使用
void MoveTo(int x, int y)
{
	_pointx = x;
	_pointy = y;
}

TypeXY GetXY(void)
{

	TypeXY m;
	m.x = _pointx;
	m.y = _pointy;
	return m;
}
int GetX(void)
{
	return _pointx;
}
int GetY(void)
{
	return _pointy;
}
void LineTo(int x, int y)
{
	DrawLine(_pointx, _pointy, x, y);
	_pointx = x;
	_pointy = y;
}
///////////////////////////////////////////////////////////////////////////////////
//绘制一个点
void DrawPixel(int x, int y)
{
	SetPointBuffer(x, y, GetDrawColor());
}
//得到某个点的颜色
Type_color GetPixel(int x, int y)
{
	if (GetPointBuffer(x, y))
		return pix_white;
	else
		return pix_black;
}

////////////////////////////////////////////////////////////////////////////////////
//划线
//参数:起点坐标 终点坐标
void DrawLine(int x1, int y1, int x2, int y2)
{
	unsigned short us;
	unsigned short usX_Current, usY_Current;

	int lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance;
	int lIncrease_X, lIncrease_Y;

	lDelta_X = x2 - x1; //计算坐标增量
	lDelta_Y = y2 - y1;

	usX_Current = x1;
	usY_Current = y1;

	if (lDelta_X > 0)
		lIncrease_X = 1; //设置单步正方向
	else if (lDelta_X == 0)
		lIncrease_X = 0; //垂直线
	else
	{
		lIncrease_X = -1; //设置单步反方向
		lDelta_X = -lDelta_X;
	}

	//Y轴的处理方式与上图X轴的处理方式同理
	if (lDelta_Y > 0)
		lIncrease_Y = 1;
	else if (lDelta_Y == 0)
		lIncrease_Y = 0; //水平线
	else
	{
		lIncrease_Y = -1;
		lDelta_Y = -lDelta_Y;
	}

	//选取不那么陡的方向依次画点
	if (lDelta_X > lDelta_Y)
		lDistance = lDelta_X;
	else
		lDistance = lDelta_Y;

	//依次画点 进入缓存区 画好后再刷新缓冲区就好啦
	for (us = 0; us <= lDistance + 1; us++) //画线输出
	{
		SetPointBuffer(usX_Current, usY_Current, GetDrawColor()); //画点
		lError_X += lDelta_X;
		lError_Y += lDelta_Y;

		if (lError_X > lDistance)
		{
			lError_X -= lDistance;
			usX_Current += lIncrease_X;
		}

		if (lError_Y > lDistance)
		{
			lError_Y -= lDistance;
			usY_Current += lIncrease_Y;
		}
	}
}

/**
 * @brief 	快速绘制一条水平线 w为长度 ------>
 * 
 * @param[in] x 起始横坐标
 * @param[in] y 起始纵坐标
 * @param[in] w 线宽
 * 
 * @example		DrawFastHLine(0, 63, 128)
 */
void DrawFastHLine(int x, int y, unsigned char w)
{
	int end = x + w;
	int a;

	Type_color color = GetDrawColor();
	for (a = MAX(0, x); a < MIN(end, SCREEN_COLUMN); a++)
	{
		SetPointBuffer(a, y, color);
	}
}

/**
 * @brief 	快速绘制一条垂直线 
 * 
 * @param[in] x 起始横坐标
 * @param[in] y 其实纵坐标
 * @param[in] h 高度
 * 
 * @example		 DrawFastVLine(127, 0, 64)
 */
void DrawFastVLine(int x, int y, unsigned char h)
{
	int end = y + h;
	int a;

	Type_color color = GetDrawColor();
	for (a = MAX(0, y); a < MIN(end, SCREEN_ROW); a++)
	{
		SetPointBuffer(x, a, color);
	}
}

/**
 * @brief 	连续画线 折线 num 为折点个数
 * 
 * @param[in] points 	坐标数组
 * @param[in] num 		坐标数量
 * 
 * @example	  TypeXY Point[5] = {{0, 63}, {8, 32}, {16, 63}, {64, 32}, {127, 63}};	
 * 			  DrawPolyLineTo(Point, sizeof(Point) / sizeof(TypeXY));	
 */
void DrawPolyLineTo(const TypeXY *points, int num)
{
	int i = 0;
	MoveTo(points[0].x, points[0].y);
	for (i = 1; i < num; i++)
	{
		LineTo(points[i].x, points[i].y);
	}
}

// * 绘制矩形
///////////////////////////////////////////////////////////////////////////////////////
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
void DrawRect1(int left, int top, int right, int bottom)
{
	DrawLine(left, top, right, top);
	DrawLine(left, bottom, right, bottom);
	DrawLine(left, top, left, bottom);
	DrawLine(right, top, right, bottom);
}

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
void DrawFillRect1(int left, int top, int right, int bottom)
{
	DrawRect1(left, top, right, bottom);
	FillRect(left + 1, top + 1, right - left - 1, bottom - top - 1);
}

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
void DrawRect2(int left, int top, int width, int height)
{
	DrawLine(left, top, left + width - 1, top);
	DrawLine(left, top + height - 1, left + width - 1, top + height - 1);
	DrawLine(left, top, left, top + height - 1);
	DrawLine(left + width - 1, top, left + width - 1, top + height - 1);
}

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
void DrawFillRect2(int left, int top, int width, int height)
{
	//先用上面的函数画外框
	DrawRect2(left, top, width, height);
	//然后填充实心
	FillRect(left + 1, top + 1, width - 1, height - 1);
}

// * 圆形绘制
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 				绘制一个圆
 * 
 * @param[in] usX_Center 	中心点坐标 x
 * @param[in] usY_Center 	中心点坐标 x
 * @param[in] usRadius 		半径
 * 
 * @example				DrawCircle(16, 8, 8);
 */
void DrawCircle(int usX_Center, int usY_Center, int usRadius)
{
	short sCurrentX, sCurrentY;
	short sError;
	sCurrentX = 0;
	sCurrentY = usRadius;
	sError = 3 - (usRadius << 1); //判断下个点位置的标志

	while (sCurrentX <= sCurrentY)
	{
		//此处画圆打点的方法和画圆角矩形的四分之一圆弧的函数有点像
		SetPointBuffer(usX_Center + sCurrentX, usY_Center + sCurrentY, GetDrawColor()); //1，研究对象
		SetPointBuffer(usX_Center - sCurrentX, usY_Center + sCurrentY, GetDrawColor()); //2
		SetPointBuffer(usX_Center - sCurrentY, usY_Center + sCurrentX, GetDrawColor()); //3
		SetPointBuffer(usX_Center - sCurrentY, usY_Center - sCurrentX, GetDrawColor()); //4
		SetPointBuffer(usX_Center - sCurrentX, usY_Center - sCurrentY, GetDrawColor()); //5
		SetPointBuffer(usX_Center + sCurrentX, usY_Center - sCurrentY, GetDrawColor()); //6
		SetPointBuffer(usX_Center + sCurrentY, usY_Center - sCurrentX, GetDrawColor()); //7
		SetPointBuffer(usX_Center + sCurrentY, usY_Center + sCurrentX, GetDrawColor()); //0
		sCurrentX++;
		if (sError < 0)
			sError += 4 * sCurrentX + 6;
		else
		{
			sError += 10 + 4 * (sCurrentX - sCurrentY);
			sCurrentY--;
		}
	}
}

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
void DrawFillCircle(int usX_Center, int usY_Center, int r)
{
	DrawFastVLine(usX_Center, usY_Center - r, 2 * r + 1);
	DrawFillCircleHelper(usX_Center, usY_Center, r, 3, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
void DrawCircleHelper(int x0, int y0, unsigned char r, unsigned char cornername)
{
	int f = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x = 0;
	int y = r;

	Type_color color = GetDrawColor();
	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x8) //右上
		{
			// 交替打点发, 二倍速画空心圆
			// x < y 退出循环
			SetPointBuffer(x0 + x, y0 + y, color);
			SetPointBuffer(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) //右下
		{
			SetPointBuffer(x0 + x, y0 - y, color);
			SetPointBuffer(x0 + y, y0 - x, color);
		}
		if (cornername & 0x4) //左下
		{
			SetPointBuffer(x0 - y, y0 + x, color);
			SetPointBuffer(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) //左上
		{
			SetPointBuffer(x0 - y, y0 - x, color);
			SetPointBuffer(x0 - x, y0 - y, color);
		}
	}
}

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
void DrawFillCircleHelper(int x0, int y0, unsigned char r, unsigned char cornername, int delta)
{
	int f = 1 - r;
	int ddF_x = 1;
	int ddF_y = -2 * r;
	int x = 0;
	int y = r;

	Type_color color = GetDrawColor();
	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) //填充右边的2个四分之一圆和中间的矩形
		{
			DrawFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta);
			DrawFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta);
		}
		if (cornername & 0x2) //填充左边的2个四分之一圆和中间的矩形
		{
			DrawFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta);
			DrawFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta);
		}
	}
}

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
void DrawArc(int x, int y, unsigned char r, int angle_start, int angle_end)
{
	float i = 0;

	TypeXY m, temp;
	temp = GetXY();
	SetRotateCenter(x, y);
	SetAnggleDir(0);
	if (angle_end > 360)
		angle_end = 360;
	SetAngle(0);
	m = GetRotateXY(x, y + r);
	MoveTo(m.x, m.y);
	for (i = angle_start; i < angle_end; i += 5)
	{
		SetAngle(i);
		m = GetRotateXY(x, y + r);
		LineTo(m.x, m.y);
	}
	LineTo(x + r, y);
	MoveTo(temp.x, temp.y);
}

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
void DrawFillArc(int x, int y, unsigned char r, int angle_start, int angle_end)
{
	return;
}

// * 圆角矩形
//////////////////////////////////////////////////////////////////////////////////////////////////
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
void DrawRoundRect(int x, int y, unsigned char w, unsigned char h, unsigned char r)
{
	//画出边缘 因为边缘是直线 所以专用高效率函数
	DrawFastHLine(x + r, y, w - 2 * r);			// Top
	DrawFastHLine(x + r, y + h - 1, w - 2 * r); // Bottom
	DrawFastVLine(x, y + r, h - 2 * r);			// Left
	DrawFastVLine(x + w - 1, y + r, h - 2 * r); // Right
	//画出四个圆角
	DrawCircleHelper(x + r, y + r, r, 1);
	DrawCircleHelper(x + w - r - 1, y + r, r, 2);
	DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 8);
	DrawCircleHelper(x + r, y + h - r - 1, r, 4);
}

/**
 * @brief           绘制一个填充圆角矩形
 * 
 * @param[in] x     横坐标         
 * @param[in] y     纵坐标
 * @param[in] w     长
 * @param[in] h     宽
 * @param[in] r     弧度
 * 
 * @example         DrawfillRoundRect(2, 2, 125, 62, 15);
 */
void DrawfillRoundRect(int x, int y, unsigned char w, unsigned char h, unsigned char r)
{
	//画实心矩形
	DrawFillRect2(x + r, y, w - 2 * r, h);

	//再填充左右两边
	DrawFillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1); //右上角的圆心
	DrawFillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1);		 //左下角的圆心
}

// * 椭圆
/////////////////////////////////////////////////////////////////////////////////////////////////////
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
void DrawEllipse(int xCenter, int yCenter, int Rx, int Ry)
{
	int Rx2 = Rx * Rx;
	int Ry2 = Ry * Ry;
	int twoRx2 = 2 * Rx2;
	int twoRy2 = 2 * Ry2;
	int p;
	int x = 0;
	int y = Ry;
	int px = 0;
	int py = twoRx2 * y;

	//椭圆最下面的点
	SetPointBuffer(xCenter + x, yCenter + y, GetDrawColor()); //因为此时x=0 俩个点为同一个 原作这样写的 那就这样吧
	SetPointBuffer(xCenter - x, yCenter + y, GetDrawColor());
	//椭圆最上面的点
	SetPointBuffer(xCenter + x, yCenter - y, GetDrawColor());
	SetPointBuffer(xCenter - x, yCenter - y, GetDrawColor());

	//Region?1 画出上下的线条 说实话我也没看懂了 算法大佬
	p = (int)(Ry2 - Rx2 * Ry + 0.25 * Rx2);
	while (px < py)
	{
		x++;
		px += twoRy2;
		if (p < 0)
			p += Ry2 + px;
		else
		{
			y--;
			py -= twoRx2;
			p += Ry2 + px - py;
		}
		SetPointBuffer(xCenter + x, yCenter + y, GetDrawColor());
		SetPointBuffer(xCenter - x, yCenter + y, GetDrawColor());
		SetPointBuffer(xCenter + x, yCenter - y, GetDrawColor());
		SetPointBuffer(xCenter - x, yCenter - y, GetDrawColor());
	}

	//Region?2
	p = (int)(Ry2 * (x + 0.5) * (x + 0.5) + Rx2 * (y - 1) * (y - 1) - Rx2 * Ry2);
	while (y > 0)
	{
		y--;
		py -= twoRx2;
		if (p > 0)
			p += Rx2 - py;
		else
		{
			x++;
			px += twoRy2;
			p += Rx2 - py + px;
		}
		SetPointBuffer(xCenter + x, yCenter + y, GetDrawColor());
		SetPointBuffer(xCenter - x, yCenter + y, GetDrawColor());
		SetPointBuffer(xCenter + x, yCenter - y, GetDrawColor());
		SetPointBuffer(xCenter - x, yCenter - y, GetDrawColor());
	}
}
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
void DrawFillEllipse(int x0, int y0, int rx, int ry)
{
	int x, y;
	int xchg, ychg;
	int err;
	int rxrx2;
	int ryry2;
	int stopx, stopy;

	rxrx2 = rx;
	rxrx2 *= rx;
	rxrx2 *= 2;

	ryry2 = ry;
	ryry2 *= ry;
	ryry2 *= 2;

	x = rx;
	y = 0;

	xchg = 1;
	xchg -= rx;
	xchg -= rx;
	xchg *= ry;
	xchg *= ry;

	ychg = rx;
	ychg *= rx;

	err = 0;

	stopx = ryry2;
	stopx *= rx;
	stopy = 0;

	while (stopx >= stopy)
	{
		DrawFastVLine(x0 + x, y0 - y, y + 1);
		DrawFastVLine(x0 - x, y0 - y, y + 1);
		DrawFastVLine(x0 + x, y0, y + 1);
		DrawFastVLine(x0 - x, y0, y + 1);
		y++;
		stopy += rxrx2;
		err += ychg;
		ychg += rxrx2;
		if (2 * err + xchg > 0)
		{
			x--;
			stopx -= ryry2;
			err += xchg;
			xchg += ryry2;
		}
	}

	x = 0;
	y = ry;

	xchg = ry;
	xchg *= ry;

	ychg = 1;
	ychg -= ry;
	ychg -= ry;
	ychg *= rx;
	ychg *= rx;

	err = 0;
	stopx = 0;
	stopy = rxrx2;
	stopy *= ry;

	while (stopx <= stopy)
	{
		DrawFastVLine(x0 + x, y0 - y, y + 1);
		DrawFastVLine(x0 - x, y0 - y, y + 1);
		DrawFastVLine(x0 + x, y0, y + 1);
		DrawFastVLine(x0 - x, y0, y + 1);

		x++;
		stopx += ryry2;
		err += xchg;
		xchg += ryry2;
		if (2 * err + ychg > 0)
		{
			y--;
			stopy -= rxrx2;
			err += ychg;
			ychg += rxrx2;
		}
	}
}
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
void DrawEllipseRect(int x0, int y0, int x1, int y1)
{
	int a = abs(x1 - x0);
	int b = abs(y1 - y0); //get diameters
	int b1 = b & 1;
	long dx = 4 * (1 - a) * b * b;
	long dy = 4 * (b1 + 1) * a * a;
	long err = dx + dy + b1 * a * a;
	long e2;

	if (x0 > x1)
	{
		x0 = x1;
		x1 += a;
	}
	if (y0 > y1)
	{
		y0 = y1;
	}
	y0 += (b + 1) / 2;
	y1 = y0 - b1;
	a *= 8 * a;
	b1 = 8 * b * b;

	do
	{
		DrawPixel(x1, y0);
		DrawPixel(x0, y0);
		DrawPixel(x0, y1);
		DrawPixel(x1, y1);
		e2 = 2 * err;
		if (e2 >= dx)
		{
			x0++;
			x1--;
			err += dx += b1;
		}
		if (e2 <= dy)
		{
			y0++;
			y1--;
			err += dy += a;
		}
	} while (x0 <= x1);

	while (y0 - y1 < b)
	{
		DrawPixel(x0 - 1, y0);
		DrawPixel(x1 + 1, y0++);
		DrawPixel(x0 - 1, y1);
		DrawPixel(x1 + 1, y1--);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// * 画三角形
/**
 * @brief           三点绘制三角形
 * 
 * @param[in] xi     第i个点 x   
 * @param[in] yi     第i个点 y
 * 
 * @example         DrawTriangle(10, 10, 70, 40, 90, 40);
 */
void DrawTriangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
	//很简单  就是画3条任意线
	DrawLine(x0, y0, x1, y1);
	DrawLine(x1, y1, x2, y2);
	DrawLine(x2, y2, x0, y0);
}
/**
 * @brief           三点绘制填充三角形
 * 
 * @param[in] xi     第i个点 x   
 * @param[in] yi     第i个点 y
 * 
 * @example         DrawFillTriangle(10, 10, 70, 40, 90, 40);
 */
void DrawFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	int a, b, y, last;
	int dx01, dy01, dx02, dy02, dx12, dy12, sa = 0, sb = 0;

	Type_color tmpcolor;
	tmpcolor = GetDrawColor();
	SetDrawColor(GetFillColor());
	if (y0 > y1)
	{
		SWAP(y0, y1);
		SWAP(x0, x1);
	}
	if (y1 > y2)
	{
		SWAP(y2, y1);
		SWAP(x2, x1);
	}
	if (y0 > y1)
	{
		SWAP(y0, y1);
		SWAP(x0, x1);
	}
	if (y0 == y2)
	{
		a = b = x0;
		if (x1 < a)
		{
			a = x1;
		}
		else if (x1 > b)
		{
			b = x1;
		}
		if (x2 < a)
		{
			a = x2;
		}
		else if (x2 > b)
		{
			b = x2;
		}
		DrawFastHLine(a, y0, b - a + 1);
		return;
	}
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1,
	sa = 0,
	sb = 0;
	if (y1 == y2)
	{
		last = y1; // Include y1 scanline
	}
	else
	{
		last = y1 - 1; // Skip it
	}

	for (y = y0; y <= last; y++)
	{
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;

		if (a > b)
		{
			SWAP(a, b);
		}
		DrawFastHLine(a, y, b - a + 1);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++)
	{
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if (a > b)
		{
			SWAP(a, b);
		}
		DrawFastHLine(a, y, b - a + 1);
	}
	SetDrawColor(tmpcolor);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
void DrawBitmap(int x, int y, const unsigned char *bitmap, unsigned char w, unsigned char h)
{
	int iCol, a;
	int yOffset = abs(y) % 8;
	int sRow = y / 8;
	int rows = h / 8;

	if (x + w < 0 || x > SCREEN_COLUMN - 1 || y + h < 0 || y > SCREEN_ROW - 1)
		return;
	if (y < 0)
	{
		sRow--;
		yOffset = 8 - yOffset;
	}

	if (h % 8 != 0)
		rows++;
	for (a = 0; a < rows; a++)
	{
		int bRow = sRow + a;
		if (bRow > (SCREEN_ROW / 8) - 1)
			break;
		if (bRow > -2)
		{
			for (iCol = 0; iCol < w; iCol++)
			{
				if (iCol + x > (SCREEN_COLUMN - 1))
					break;
				if (iCol + x >= 0)
				{
					if (bRow >= 0)
					{
						if (GetDrawColor() == pix_white)
						{
							unsigned char temp = ReadByteBuffer(bRow, x + iCol);
							temp |= pgm_read_byte(bitmap + (a * w) + iCol) << yOffset;
							WriteByteBuffer(bRow, x + iCol, temp);
						}
						else if (GetDrawColor() == pix_black)
						{
							unsigned char temp = ReadByteBuffer(bRow, x + iCol);
							temp &= ~(pgm_read_byte(bitmap + (a * w) + iCol) << yOffset);
							WriteByteBuffer(bRow, x + iCol, temp);
						}
						else
						{
							unsigned char temp = ReadByteBuffer(bRow, x + iCol);
							temp ^= (pgm_read_byte(bitmap + (a * w) + iCol) << yOffset);
							WriteByteBuffer(bRow, x + iCol, temp);
						}
					}
					if (yOffset && bRow < (SCREEN_ROW / 8) - 1 && bRow > -2)
					{
						if (GetDrawColor() == pix_white)
						{
							unsigned char temp = ReadByteBuffer(bRow + 1, x + iCol);
							temp |= pgm_read_byte(bitmap + (a * w) + iCol) >> (8 - yOffset);
							WriteByteBuffer(bRow + 1, x + iCol, temp);
						}
						else if (GetDrawColor() == pix_black)
						{
							unsigned char temp = ReadByteBuffer(bRow + 1, x + iCol);
							temp &= ~(pgm_read_byte(bitmap + (a * w) + iCol) >> (8 - yOffset));
							WriteByteBuffer(bRow + 1, x + iCol, temp);
						}
						else
						{
							unsigned char temp = ReadByteBuffer(bRow + 1, x + iCol);
							temp ^= pgm_read_byte(bitmap + (a * w) + iCol) >> (8 - yOffset);
							WriteByteBuffer(bRow + 1, x + iCol, temp);
						}
					}
				}
			}
		}
	}
}

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
void DrawXBitmap(int x, int y, const unsigned char *bitmap, unsigned char w, unsigned char h)
{
	// no need to dar at all of we're offscreen
	int xi, yi, byteWidth = (w + 7) / 8;
	if (x + w < 0 || x > SCREEN_COLUMN - 1 || y + h < 0 || y > SCREEN_ROW - 1)
		return;
	for (yi = 0; yi < h; yi++)
	{
		for (xi = 0; xi < w; xi++)
		{
			if (pgm_read_byte(bitmap + yi * byteWidth + xi / 8) & (1 << (xi & 7)))
			{
				SetPointBuffer(x + xi, y + yi, GetDrawColor());
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief           绘制一个字符
 * 
 * @param[in] x y   开始坐标     
 * @param[in] c     字符
 * @note            当size=0时 x为0-7行 y为0-127列
 * 
 * @example			Draw(0,0, 'C');
 * 
 * @note			
 */
void DrawChar(int x, int y, unsigned char c)
{
	int i, j;
	unsigned char draw_background, bg, a, b, size, color;

	size = GetFontSize();		   //字体尺寸
	color = GetDrawColor();		   //字体颜色 1白0黑
	bg = GetTextBkMode();		   //写字的时候字的背景的颜色 1白0黑
	draw_background = bg != color; //这两个颜色要不一样字才看得到

	if (!size) //默认字符大小
	{
		if ((x > 6) || (y > SCREEN_COLUMN - 8))
			return;
		c = c - ' '; //得到偏移后的位置
		for (i = 0; i < 8; i++)
			WriteByteBuffer(x, y + i, F8X16[c * 16 + i]);
		for (i = 0; i < 8; i++)
			WriteByteBuffer(x + 1, y + i, F8X16[c * 16 + i + 8]);
	}
	else //使用原作粗体字符
	{
		//判断一个字符的上下左右是否超出边界范围
		if ((x >= SCREEN_COLUMN) ||		// Clip right
			(y >= SCREEN_ROW) ||		// Clip bottom
			((x + 5 * size - 1) < 0) || // Clip left
			((y + 8 * size - 1) < 0)	// Clip top
		)
			return;

		for (i = 0; i < 6; i++)
		{
			int line;
			//一个字符在font5x7中由一行6个char表示
			//line为这个字符的第某行内容
			if (i == 5)
				line = 0x0;
			else
				line = pgm_read_byte(font5x7 + (c * 5) + i);

			for (j = 0; j < 8; j++)
			{
				unsigned char draw_color = (line & 0x1) ? color : bg; //目前需要填充的颜色是0 就是背景色 1就是字体色

				//不同号大小的字体只是最基础字体的放大倍数 这点要注意
				//比如基础字是1个像素 放大后就是4个像素 再就是9个像素 达到马赛克的放大效果
				if (draw_color || draw_background)
					for (a = 0; a < size; a++)
						for (b = 0; b < size; b++)
							SetPointBuffer(x + (i * size) + a, y + (j * size) + b, draw_color);

				line >>= 1;
			}
		}
	}
}

/**
 * @brief           绘制字符串
 * 
 * @param[in] x y   开始坐标     
 * @param[in] str   字符
 * 
 * @example			DrawString(0,0, blacksheep);
 * 
 * @note            当size=0时 x为0-7行 y为0-127列, 一行最大 16个字符串
 */			
void DrawString(int x, int y, char *str)
{
	unsigned char j = 0, tempx = x, tempy = y;
	unsigned char size = GetFontSize();

	if (!size) //默认字体
	{
		while (str[j] != '\0')
		{
			DrawChar(x, y, str[j]);
			y += 8;
			if (y > 120)
			{
				y = 0;
				x += 2;
			}
			j++;
		}
	}
	else //使用原作粗体字符
	{
		while (str[j] != '\0')
		{
			if (str[j] == '\n')
			{
				tempy += 8 * size;
				tempx = x;
				j++;
				continue;
			}
			DrawChar(tempx, tempy, str[j]);
			tempx += size * 6;
			j++;
		}
	}
}

/**
 * @brief           绘制数字
 * 
 * @param[in] x y   开始坐标     
 * @param[in] num   数字
 * @param[in] len   长度
 * @note            当size=0时 x为0-7行 y为0-127列
 */
void DrawNum(unsigned char x, unsigned char y, unsigned int num, unsigned char len)
{
	unsigned char t, temp;
	unsigned char enshow = 0;
	unsigned char size = GetFontSize();

	if (!size)
	{
		for (t = 0; t < len; t++)
		{
			temp = (num / oled_pow(10, len - t - 1)) % 10;
			if (enshow == 0 && t < (len - 1))
			{
				if (temp == 0)
				{
					DrawChar(x, y + 8 * t, ' ');
					continue;
				}
				else
					enshow = 1;
			}
			DrawChar(x, y + 8 * t, temp + '0');
		}
	}
	else
	{
		for (t = 0; t < len; t++)
		{
			temp = (num / oled_pow(10, len - t - 1)) % 10;
			if (enshow == 0 && t < (len - 1))
			{
				if (temp == 0)
				{
					DrawChar(x + (size * 6) * t, y, '0');
					continue;
				}
				else
					enshow = 1;
			}
			DrawChar(x + (size * 6) * t, y, temp + '0');
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 			显示中文, ShowChinese 底层为这个函数
 * 
 * @param[in] str 	中文
 * @param[in] len 	字符串长度
 * 
 * @note			汉字需要使用PCtoLCD2002生成, 并添加到 oled_font.c文件中
 * !				字模大小为 16,所以
 * 					x 值为 0~3,
 * 					y 值为 0~7
 */
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t *cn)
{
	uint8_t j, wordNum;

	if ((x > 7) || (y > 128 - 16))
		return;

	while (*cn != '\0') //在C语言中字符串结束以‘\0’结尾
	{
		for (wordNum = 0; wordNum < NUM_OFCHINESE; wordNum++)
		{
			if ((CN16CHAR[wordNum].Index[0] == *cn) && (CN16CHAR[wordNum].Index[1] == *(cn + 1))) //查询要写的字在字库中的位置
			{
				for (j = 0; j < 32; j++) //写一个字
				{
					if (j == 16) //由于16X16用到两个Y坐标，当大于等于16时，切换坐标
					{
						x++;
					}
					WriteByteBuffer(x, y + (j % 16), CN16CHAR[wordNum].Msk[j]);
				}
				y += 16;
				x--;
				if (y > (128 - 16))
				{
					x += 2;
					y = 0;
				}
			}
		}
		cn += 2; //此处打完一个字，接下来寻找第二个字
	}
}

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
void ShowChinese(int x, int y, const char *str)
{
	uint8_t buf[4] = "";
	x += x;
	while (*str != '\0')
	{
		strncpy((char *)buf, str, 3);
		OLED_ShowCHinese(x, y++ * 16, buf);
		if (x < 8 && y > 7)
		{ // 换行
			y = 0;
			x += 2;
		}
		str += 3;
	}
}