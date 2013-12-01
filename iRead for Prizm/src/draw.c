/*

iRead II for Prizm Pro
Drawing library source file

(c)2013 ExAcler & wtof1996 Some rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <display.h>

void draw_pic (int x,int y,int width,int height,int sel,unsigned char* pimage)
{
    /*
	    黑白图片绘制 (图片数据结构详见 fx-9860G Libraries)
	    参数说明：
		    x: 绘制图片的左上角横坐标
			y: 绘制图片的左上角纵坐标
			width: 图片的宽度
			height: 图片的高度
			sel: 像素点绘制的颜色
			pimage: 图片数据
	*/

	int i,j,k,pixel,rx=0,ry=0;    //  rx,ry: 屏幕上的像素绘制横/纵坐标
	unsigned char p;
    int iwidth = width/8+(width%8?1:0);    //  取得图片实际字节数，一个字节存储 8 个像素，若总宽度不满 8，补至 8 的整数倍

	for (i=0;i<height;++i,pimage+=iwidth)    //  逐行读取
	{
		ry=y+i;

		for (j=0;j<iwidth;++j)    //  列上每次跳 8 个字节进行读取
		{
			p = pimage[j];
			for (k=0;k<8;++k)    //  将 1 个字节拆分为 8 位进行读取
			{
				rx=x+(8-k)+8*j;    //  取得像素在屏幕上的位置
	 			pixel=p%2;    //  取得像素的颜色信息
				p>>=1;    //  右移，读取下一位
	 			if (pixel) Bdisp_SetPoint_VRAM(rx,ry,sel);    //  绘制像素点
			}
		}
		
	}
}

void Bdisp_AreaReverseVRAM(int x1,int y1,int x2,int y2)
{
    /*
	    区域反白
	    参数说明：
		    x1: 反白区域的左上角横坐标
			y1: 反白区域的左上角纵坐标
			x1: 反白区域的右下角横坐标
			y1: 反白区域的右下角纵坐标
	*/

    int i,j;
	for (i=x1;i<x2;++i)
	    for (j=y1;j<y2;++j)
		    Bdisp_SetPoint_VRAM(i,j,Bdisp_GetPoint_VRAM(i,j) ^ 0xFFFF);    //  对像素点进行全白异或即可实现反白效果
}

void ClearArea(int x1,int y1,int x2,int y2)
{
    /*
	    区域清空
	    参数说明：
		    x1: 清空区域的左上角横坐标
			y1: 清空区域的左上角纵坐标
			x1: 清空区域的右下角横坐标
			y1: 清空区域的右下角纵坐标
	*/
   
    int i,j;

    for (i=y1;i<y2;i++)
        for (j=x1;j<x2;j++)
            Bdisp_SetPoint_VRAM(j,i,0xFFFF);    //  将像素点设为白色即可实现清空效果

}

int print_chs_page(int x,int y,const int totbytes,const unsigned char* str)
{
    /*
	    绘制一页，注释详见 divide_page 函数
	*/

    int cx=x;
    int cy=y;
    int i=0;
    const int pp=16;
	int is_chs=0;
    char temp[50];
    while (1)
    {
	    if (str[i]==0) return -1;
        is_chs=str[i] & 0x80;
        if ((cx+pp)>368)
            goto cn;
        if (is_chs)
        {
            print_chs_char(cx,cy,0,str[i],str[i+1]);
            i+=2;
        }
        else
        {
		    if (str[i]=='\n')
			{
			    i++;
				goto cn;
			}
            print_asc_char (cx,cy,0,str[i]);
			i++;
        }
        
		if (is_chs)
            cx+=25;
		else
		    cx+=16;
        
        if (cx>368)
        {
        cn:
            cx=x;
            cy+=24;
			if (cy>190)
			    goto end;
        }
    }
	end:
	return i;
}