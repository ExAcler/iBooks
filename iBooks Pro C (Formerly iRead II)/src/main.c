/*

iBooks Pro C
Program main source file

(c)2013 - 2017 Xhorizon, Some rights reserved.

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

#include <display_syscalls.h>
#include <keyboard_syscalls.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "fxchslib.h"
#include "browser.h"
#include "keybios.h"

int Bfile_OpenFile_OS	(const unsigned short*filename, int mode );
int Bfile_CloseFile_OS	(int HANDLE );
int Bfile_ReadFile_OS	(int HANDLE, void *buf, int size, int readpos );
int Bfile_GetFileSize_OS( int handle, int pos );

int Bfile_WriteFile_OS( int HANDLE, const void *buf, int size );
int Bfile_CreateEntry_OS( const unsigned short*filename, int mode, int*size );
int Bfile_SeekFile_OS( int handle, int pos );

FONTFILE* font16;
int totbytes=0,page=0,cached=0;    // 读取字节指针、当前页码、已缓存页数
unsigned int bytes[9999];    // 页面缓存
unsigned int bookmark[3];    // 书签

int divide_page(const char* filename,const int pages,int save)
{
    /*
	    分页函数
        参数说明：
	        filename: 当前打开文件的文件名
            pages: 需要分的页数
            save: 是否需要跳回第一页 (初始化时使用)
		返回值：
		    0: 已读到文件末尾
			1: 未读到文件末尾
    */

    int cx,cy;
    int i=0,j;
    const int pp=16;
	int is_chs=0;  // 中文字符标识
	int tmp=cached,total=0;
	
	int decades_passed = 1;
	
	int handle;
	char* buf=(char*)malloc(460);
	FONTCHARACTER fname[64];
	
    char_to_font(filename,fname);
	handle=Bfile_OpenFile_OS(fname,0);
	tmp=cached;
	totbytes=bytes[cached];  // 保险修正
	
	// 如果请求的页数超过 9999 页，修正为 9999 页
	if ((total=tmp+pages)>9999) total=9999;
	// 在 9999 页时尝试往后翻一页，跳出
	if (cached+1>9999) {Bfile_CloseFile_OS(handle);return 0;};
	
	// 在屏幕上显示分页的进度
	ProgressBar(0, total - tmp);
	
	for (j=tmp;j<total;++j)    //  从当前已缓存页面分到请求的页面，使用模拟填充显示区域方法
	{
	    // 尝试读取一段文字以备分页
	    memset(buf,0,461);
	    Bfile_ReadFile_OS(handle,buf,400,totbytes);
		// 如果读到文件末尾则跳出
		if (!buf[0])
		{
			ProgressBar(total - tmp, total - tmp);
			Bfile_CloseFile_OS(handle);
			MsgBoxPop();
			return 0;
		}
		
		// 填充位置设置为初始状态
        cx=0;
		cy=24;
		
        for (i=0;;)
        {
            is_chs=buf[i] & 0x80;  // 判断高位字节，确定是否为中文
            if ((cx+pp)>368)  // 填充超过屏幕右边缘
                goto cn;
            if (is_chs) i+=2;  // 中文，跳2字节
            else
            {
		        if (buf[i] == '\r' || buf[i] == '\n')  // 若读到回车符直接进入下一行
			    {
			        i++;
					if (buf[i] == '\r' || buf[i] == '\n')  // 若读到回车符直接进入下一行
						i++;
				    goto cn;
			    }
			    i++;
            }
        
		    // 将字符填充入当前行，增加字符偏移
		    if (is_chs)
                cx+=25;
		    else
		        cx+=18;
        
		    // 填充超过屏幕右边缘
            if (cx>368)
            {
            cn:
                cx=0;
                cy+=24;  // 增加纵向字符偏移，进入下一行
			    if (cy>190)  // 填充超过屏幕下边缘，跳出
			        break;
            }
        }
	    bytes[j+1]=i+totbytes;  // 将最后一个字符在文件中的位置存入下一页的缓存，以备读取
		totbytes+=i;  // 读取字节指针增加
	    ++cached;  // 已缓存页面增加，表示分页成功
		
		// 每分完1/10的总体，增加进度显示
		if (j - tmp == (total - tmp) / 10 * decades_passed)
		{
			ProgressBar(j - tmp, total - tmp);
			decades_passed++;
		}
	}
	if (save) page=0;    // 跳回第一页  
	
	ProgressBar(total - tmp, total - tmp);
	Bfile_CloseFile_OS(handle);
	MsgBoxPop();
	return 1;
}

void Save_Config(const char* fn,int n)
{
    /*
	    存储分页及书签配置
		参数说明：
		    fn: 当前打开文件的文件名
		    n: 已缓存页数
	*/

    char tmp[64];
	FONTCHARACTER fname[64];
	int handle,size=0;

    memset(tmp,0,sizeof(tmp));
	strncpy(tmp,fn,strlen(fn)-strlen(strrchr(fn,'.')));    // 取文件名部分
	strcat(tmp,".cfg");
	
	char_to_font(tmp,fname);
	Bfile_CreateEntry_OS(fname,1,&size);    // 创建 .cfg 文件
	handle=Bfile_OpenFile_OS(fname,2);
	if (handle<=0) return;
	
	Bfile_WriteFile_OS(handle,&n,4);    // 前 4 字节，写入已缓存页数
	
	Bfile_SeekFile_OS(handle,4);
	Bfile_WriteFile_OS(handle,bookmark,16);    // 4*4 字节，写入书签指向的页数
	
	Bfile_SeekFile_OS(handle,20);
	Bfile_WriteFile_OS(handle,bytes,n*4);    // 4*n 字节，写入页面缓存
	Bfile_CloseFile_OS(handle);
}

void Read_Config(const char* fn,int* n)
{
    /*
	    读取分页及书签配置
		参数说明：
		    fn: 当前打开文件的文件名
		    n: 接受已缓存页数的缓冲区
	*/

    char tmp[64];
	FONTCHARACTER fname[64];
	int handle,_n=*n;

    memset(tmp,0,sizeof(tmp));
	strncpy(tmp,fn,strlen(fn)-strlen(strrchr(fn,'.')));    // 取文件名部分
	strcat(tmp,".cfg");
	
	char_to_font(tmp,fname);
	handle=Bfile_OpenFile_OS(fname,0);
	*n=_n;
	if (handle<=0) return;
	Bfile_ReadFile_OS(handle,&_n,4,0);*n=_n-1;    // 读入已缓存页数
	Bfile_ReadFile_OS(handle,bookmark,16,4);    // 读入书签页码
	Bfile_ReadFile_OS(handle,bytes,_n*4,20);    //读入页面缓存
	
	Bfile_CloseFile_OS(handle);
}

void Save_Bookmark(const char* fn,unsigned int pages,int n)
{
    /*
	    存储书签（界面绘制）
		参数说明：
		    fn: 当前打开文件的文件名
		    pages: 当前所在页码
		    n: 已缓存页面数
	*/

    int handle,size,i=0;
	int sel=0,flag;
	char tip[64],tmp[64];
	
	MsgBoxPush(5);
	
	beg:
	flag=0;
	
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0,192,384,216);
	draw_pic(0,192,124,22,0,Menu_PutB);
	
	ClearArea(35,49,349,166);
	print_chs_2(38,47,0,"存储书签");
	
	close_font(font16);
	for (i=0;i<4;++i)
	{
	    memset(tmp,0,sizeof(tmp));memset(tip,0,sizeof(tip));
	    strcat(tip,"[");
	    itoa(i+1,tmp,10);strcat(tip,tmp);
		strcat(tip,"]");
        if (bookmark[i])
		{
		    strcat(tip,"     Page ");
		    memset(tmp,0,sizeof(tmp));
		    itoa(bookmark[i],tmp,10);strcat(tip,tmp);
		}
		else strcat(tip,"     Empty");
		locate_OS(3,3+i);
		Print_OS(tip,0,0);
	}

	Bdisp_AreaReverseVRAM(35,72+sel*24,349,94+sel*24);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_UP:
			    if (sel>0) --sel;
				goto beg;break;
				
			case KEY_CTRL_DOWN:
			    if (sel<3) ++sel;
				goto beg;break;
			
			case KEY_CTRL_F1:
	        case KEY_CTRL_EXE:
				bookmark[sel]=pages+1;    // 存储书签
			    goto prg;break;
			
			case KEY_CTRL_F2:
			case KEY_CTRL_DEL:
			    bookmark[sel]=0;flag=1;goto prg;break;    // 删除书签
			
			case KEY_CTRL_EXIT:
				MsgBoxPop();
			    return;
		}
	}
	
	prg:
	Save_Config(fn,n);
	
	if (flag) goto beg;
	MsgBoxPop();
}

void Read_Bookmark(const char* fn,int* pages,int* n)
{
    /*
	    读取书签（界面绘制）
		参数说明：
		    fn: 当前打开文件的文件名
		    pages: 接受当前所在页码的缓冲区
		    n: 接受已缓存页面数的缓冲区
	*/

    int handle,_n=0,_page=*pages;
	int i,sel=0;
	FONTCHARACTER fname[64];
	char tip[64],tmp[64];
	
	MsgBoxPush(5);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0,192,384,216);
	draw_pic(0,192,61,22,0,Menu_ReadB);
	
	ClearArea(35,49,349,166);
	print_chs_2(38,47,0,"读取书签");
	
	close_font(font16);
	for (i=0;i<4;++i)
	{
	    memset(tmp,0,sizeof(tmp));memset(tip,0,sizeof(tip));
	    strcat(tip,"[");
	    itoa(i+1,tmp,10);strcat(tip,tmp);
		strcat(tip,"]");
        if (bookmark[i])
		{
		    strcat(tip,"     Page ");
		    memset(tmp,0,sizeof(tmp));
		    itoa(bookmark[i],tmp,10);strcat(tip,tmp);
		}
		else strcat(tip,"     Empty");
		locate_OS(3,3+i);
		Print_OS(tip,0,0);
	}

	Bdisp_AreaReverseVRAM(35,72+sel*24,349,94+sel*24);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_UP:
			    if (sel>0) --sel;
				goto beg;break;
				
			case KEY_CTRL_DOWN:
			    if (sel<3) ++sel;
				goto beg;break;
				
	        case KEY_CTRL_EXE:
			    if (bookmark[sel])
				{
			        *pages=bookmark[sel]-1;    // 读取书签
					MsgBoxPop();
			        return;
				}
				else goto beg;break;
			
			case KEY_CTRL_F1:
			case KEY_CTRL_EXIT:
			    *pages=_page;
				MsgBoxPop();
			    return;break;
		}
	}
}

void Confirm_AllDivide(const char* fn)
{
    /*
	    尝试全部分页（界面绘制）
		参数说明：
		    fn: 当前打开文件的文件名
	*/

	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(35,45,349,166);
	print_chs_2(38,47,0,"尝试全部分页可能需要");
	print_chs_2(38,71,0,"很长时间。");
	print_chs_2(38,95,0,"确定继续？");
	
	locate_OS(6,5);
	Print_OS("[F1]:",0,0);print_chs_2(181,119,0,"是");
	locate_OS(6,6);
	Print_OS("[F6]:",0,0);print_chs_2(181,143,0,"否");
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_F1:
			    divide_page(fn,9999-cached,0);    // 尝试分到 9999 页
				return;break;
			case KEY_CTRL_F6:
				return;break;
	    }
	}
}

void Disp_About()
{
    /*
	    显示关于信息（界面绘制）
	*/

	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	MsgBoxPush(5);
	
	ClearArea(35,45,349,166);
	locate_OS(3,2);
	Print_OS("iBooks Pro C",0,0);
	print_chs_2(38, 71, 0, "版本 ");
	locate_OS(7, 3);
	Print_OS("1.50", 0, 0);
	print_chs_2(38, 95, 0, "制作：清水视野工作室");
	print_chs_2(38, 119, 0, "本程序依");
	locate_OS(10, 5);
	Print_OS("GNU GPL v3",0,0);
	print_chs_2(38, 143, 0, "协议开放源代码。");
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_EXIT:
				MsgBoxPop();
				return;break;
	    }
	}
}

void Page_Jump(const char* fn)
{
    /*
	    跳页（界面绘制）
		参数说明：
		    fn: 当前打开文件的文件名
	*/

	FONTCHARACTER fname[64];
	char tip[64],tmp[64];
	char keybuff[32];    //  接受页码的字符缓冲区
	int inspos=0,target;
	
	memset(keybuff, 0, sizeof(keybuff));
	MsgBoxPush(5);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0,192,384,216);
	draw_pic(0,192,61,22,0,Menu_Sub_Jump);
	draw_pic(63,192,61,22,0,Menu_Jump);
	
	ClearArea(35,45,349,166);
	
	print_chs_2(38,47,0,"当前页/已缓存页数：");
	
	locate_OS(3,3);
	memset(tmp,0,sizeof(tmp));memset(tip,0,sizeof(tip));
	itoa(page+1,tmp,10);strcat(tip,tmp);
    strcat(tip,"/");memset(tmp,0,sizeof(tmp));
	itoa(cached,tmp,10);strcat(tip,tmp);
	Print_OS(tip,0,0);
	
	print_chs_2(38,119,0,"输入目标页码：");
	locate_OS(3,6);
	Print_OS("[    ]",0,0);
	locate_OS(4,6);
	Print_OS(keybuff,0,0);
	
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
			case KEY_CTRL_EXIT:
				MsgBoxPop();
			    return;break;
			
			case KEY_CTRL_F2:
			    Confirm_AllDivide(fn);    // 尝试全部分页
				goto beg;break;
			
			case KEY_CTRL_DEL:
			    if (inspos>0)
				{
					keybuff[--inspos]=0;    // 最后一个字节置为 NULL，标识已删除
					goto beg;
				}
			    break;
			
			case KEY_CTRL_F1:
			case KEY_CTRL_EXE:
			    if (inspos==0) break;
				target=atoi(keybuff);
				if (target>cached)    // 如果目标位置超出已缓存页面数，尝试分页
				{
				    if (!divide_page(fn,target-cached,1)) page=cached-1;    // 若分页已达文件末尾，则将当前页面修正到最后一页
					else page=target-1;    // 否则修正当前页面为输入的目标位置
			    }
				else
				    page=target-1;    // 修正当前页面为输入的目标位置
				MsgBoxPop();
				return;break;
			
			default:
			    if (key>=0x30&&key<=0x39)    // 输入数字 0~9
				{
				    if (inspos<=3)
					{
					    if (key==0x30&&inspos==0) break;    // 尝试第一位输入 0 时跳出
						keybuff[inspos++]=key;    // 最后一个字节添加输入
						goto beg;
					}
				}
		}
	}
}

int Subdir_Open(const char* fn)
{
    /*
	    子目录文件名输入（界面绘制）
		参数说明：
		    fn: 接收从键盘输入文件名的缓冲区
	*/

	FONTCHARACTER fname[64];
	char keybuff[32];    //  接受文件名的字符缓冲区
	int inspos = 0;
	
	memset(keybuff, 0, sizeof(keybuff));
	MsgBoxPush(4);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0, 192, 384, 216);
	draw_pic(0, 192, 61, 22, 0, Menu_Sub_Jump);
	
	ClearArea(35, 45, 349, 141);
	print_chs_2(38, 71, 0, "输入欲打开的文件名：");
		
	locate_OS(3, 4);
	Print_OS("[        ].txt", 0, 0);
	locate_OS(4, 4);
	Print_OS(keybuff, 0, 0);
	
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
			case KEY_CTRL_EXIT:
				MsgBoxPop();
			    return 0; break;
						
			case KEY_CTRL_DEL:
			    if (inspos > 0)
				{
					keybuff[--inspos] = 0;    // 最后一个字节置为 NULL，标识已删除
					goto beg;
				}
			    break;
			
			case KEY_CTRL_F1:
			case KEY_CTRL_EXE:
			    strcpy(fn, keybuff);
				//MsgBoxPop();
				return 1; break;
			
			default:
			    if (key >= 0x30 && key <= 0x39 || key >= 0x41 && key <= 0x5A || key >= 0x61 && key <= 0x7A)    // 输入数字 0~9 或英文字母 a-z、A-Z
				{
				    if (inspos <= 8)
					{
						keybuff[inspos++] = key;    // 最后一个字节添加输入
						goto beg;
					}
				}
		}
	}
}

void Disp_FileNotFound()
{
    /*
	    显示“文件未找到”消息（界面绘制）
	*/

	//MsgBoxPush(4);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(35, 45, 349, 141);
	print_chs_2(94, 71, 0,"文件未找到");
	
	print_chs_2(94, 119, 0, "按");
	locate_OS(8, 5);
	Print_OS(":[EXIT]", 0, 0);
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_EXIT:
				//MsgBoxPop();
				return; break;
	    }
	}
}

void aa(int* pos,int* firstn,const int n)
{
    /*
	    列表光标上移处理函数
		参数说明：
		    pos: 当前列表光标所在位置 (0 起始)
			firstn：列表下移的行数 (0 起始)
			n: 列表的项目总数
	*/

    if ((*pos+*firstn)>0)    // 如果列表光标已经下移，上移光标
				{
				    --(*pos);    // 上移光标
					if (*pos<0)    //  如果光标移到了顶部
					{
					    *pos=0;
						--(*firstn);    // 列表整体上移
					}
				}
	else    // 如果列表光标在第一项，循环滚动
	            {
				    if (n>6)    //  如果列表项数超过一屏
					{
                        *pos=5;
					    *firstn=n-6;    //  跳到整个列表的最后一项 (底部)
					}
					else
					{
					    *pos=n-1;
						*firstn=0;    //  跳到整个列表的最后一项
					}
                }				
}

void bb(int* pos,int* firstn,const int n)
{
    /*
	    列表光标下移移处理函数
		参数说明：
		    pos: 当前列表光标所在位置 (0 起始)
			firstn：列表下移的行数 (0 起始)
			n: 列表的项目总数
	*/
	
    if ((*pos+*firstn)<n-1)    //  如果列表光标未到末尾，下移光标
				{
                    ++(*pos);    //  下移光标
					if (*pos>5)    //  如果光标移出一屏的范围
					{
					    *pos=5;    //  修正光标位置
						++(*firstn);    //  列表整体下移
					}
				}
				else    //  如果已经移到末尾，循环滚动
				{
				    *pos=0;
					*firstn=0;    //  光标回到第一项
				}
}

void iRead_main(const char* filename)
{
    /*
	    阅读界面主函数
		参数说明：
		    filename: 打开的文件名 (从文件浏览器得到)
	*/

    int key,handle;
	char* buf=(char*)malloc(461);
	FONTCHARACTER fname[64];
	
	char tip[64], tmp[64];
	
	page=0;cached=0;
	
	memset(bytes,0,sizeof(bytes));
	memset(bookmark,0,sizeof(bookmark));bookmark[3]=0;
	
	Read_Config(filename,&cached);    //  读取书签及分页配置
	
	//  如果分的页数不满 500 的整数倍，补分页满
	if (cached==0) divide_page(filename,500-cached,1);
	else
	    if (cached%500!=0) divide_page(filename,500-cached%500,1);   // 补至 500 的整数倍
	totbytes=0;
	
	/*  设置状态栏显示文字
		0x0001：显示电量
		0x0100：显示文字
	*/
	DefineStatusAreaFlags(3, 0x01 | 0x02 | 0x100, 0, 0);
	
	beg:
	font16=open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	Bdisp_AllClr_VRAM();
    draw_pic(0,192,124,22,0,Menu_Read);
	draw_pic(126,192,61,22,0,Menu_Sub_Jump);
	
	//  若翻下一页时超出已缓存页面范围
	if (cached<=page)
	{
	    //  如果分的页数不满 500 的整数倍，补分页满
	    if (!divide_page(filename,1,0)) page=cached-1;
	    else if (cached%500!=0) divide_page(filename,500-cached%500,0);
		
		close_font(font16);
		goto beg;
	}
	totbytes=bytes[page];    //  修正读取字节指针位置
	
	char_to_font(filename,fname);
	handle=Bfile_OpenFile_OS(fname,0);    //  打开文件
	
	Bfile_ReadFile_OS(handle,buf,400,totbytes);
	Bfile_CloseFile_OS(handle);
	
	print_chs_page(0,24,totbytes,(unsigned char*)buf);    //  绘制一页
    close_font(font16);
	
	//  准备显示浏览进度
	char fn_ptr[64];
	memset(fn_ptr, 0, sizeof(fn_ptr));
	GetDisplayFileName(filename, fn_ptr);
	
	memset(tip, 0, sizeof(tip));
	memset(tmp, 0, sizeof(tmp));
	strcat(tip, fn_ptr); strcat(tip, " ");
	itoa(page + 1, tmp, 10); strcat(tip, tmp);
    strcat(tip, "/"); memset(tmp, 0, sizeof(tmp));
	itoa(cached, tmp, 10);strcat(tip, tmp);
	
	//  状态栏显示文件名及进度
	DefineStatusMessage(tip, 0, 0, 0);
	
	while (1)
	{
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_UP:    //  跳到上一页
			    if (page>0)
				{
			        --page;
				    goto beg;
				}
				break;
				
		    case KEY_CTRL_DOWN:    //  跳到下一页
			    ++page;
			    goto beg;
				break;
				
			case KEY_CTRL_EXIT:    //  离开，返回文件浏览器
			    Save_Config(filename,cached+1);
				DefineStatusAreaFlags(3, 0x01 | 0x02 | 0x100, 0, 0);
			    return;break;
				
			case KEY_CTRL_F2:    //  打开存储书签对话框
			    Save_Bookmark(filename,page,cached+1);
				goto beg;break;
				
			case KEY_CTRL_F1:    //  打开读取书签对话框
			    Read_Bookmark(filename,&page,&cached);
				goto beg;break;
				
			case KEY_CTRL_F3:    //  打开跳页对话框
			    Page_Jump(filename);
				goto beg;break;
		}
 	}
}

int cmp(const void *a, const void *b)
{
	/*
	    用于文件名排序的比较函数
	*/
	char aa[100], bb[100];
	char *oo;
	strcpy(aa, ((const f_name *)a) -> name); strcpy(bb, ((const f_name *)b) -> name);
	
	if (oo = strchr(aa, '[')) *oo = ' ';
	if (oo = strchr(bb, '[')) *oo = ' ';
	
    return strcmp(aa, bb);
}

void browse_main()
{
    /*
	    文件浏览器主函数
	*/

    char ncat[64], workdir[64] = "\\\\fls0";    //  当前目录
    f_name *a=get_file_list("\\\\fls0\\*.*");    //  存储文件列表的二维数组
	int pos=0,firstn=0;    //  列表光标位置、列表下移的行数
	unsigned int key;
	char subdir_fn[32];    //  供接收子目录文件名输入的缓冲区
	FONTCHARACTER fname[64];
	int handle = 0;
	
	DefineStatusAreaFlags(3, 0x01 | 0x02 | 0x100, 0, 0);
	
	beg:
	if (a) qsort(a, getn(a), sizeof(char *), cmp);
	
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	draw_browser(workdir,firstn,pos,a);    //  绘制浏览器界面
	
	close_font(font16);
	
	//  显示当前工作目录于状态栏
	if (strcmp(workdir, "\\\\fls0") == 0)
		DefineStatusMessage("", 0, 0, 0);
	else
	{
		memset(ncat, 0, sizeof(ncat));
		GetDisplayDirName(workdir, ncat);
		DefineStatusMessage(ncat, 0, 0, 0);
	}
	
	while (1)
	{
	    GetKey(&key);
		switch (key)
		{
		    case KEY_CTRL_UP:    //  光标上移
			    if (a)
				{
				    aa(&pos,&firstn,getn(a));
					goto beg;
				}
				break;
				
			case KEY_CTRL_DOWN:    //  光标下移
			    if (a)
				{
				    bb(&pos,&firstn,getn(a));
					goto beg;
				}
				break;
			
			case KEY_CTRL_F6:    //  显示关于信息
			    Disp_About();
				goto beg;
				break;
			
			case KEY_CTRL_F1:    //  打开光标位置的文件
			case KEY_CTRL_EXE:
			    if (a)    //  如果文件列表不为空
			    {
			        if (strchr(a[pos+firstn].name,'['))    //  如果打开的是文件夹
				    {
				        memset(ncat,0,sizeof(ncat));
					    //strcat(ncat,"\\\\fls0\\");
						strcat(ncat, workdir); strcat(ncat, "\\");
				        strcat(ncat, ++a[pos+firstn].name);
					    memset(workdir, 0, sizeof(workdir));
					    strcpy(workdir, ncat);
					    strcat(ncat, "\\*.*");    //  解析出文件夹名称
					    a=get_file_list(ncat);    //  浏览该文件夹
						pos=0; firstn=0;    //  列表初始化
					    goto beg;
				    }
				    else    //  如果打开的是文本文件
				    {	
						memset(ncat,0,sizeof(ncat));
				        strcpy(ncat,workdir);
						strcat(ncat,"\\");
				        strcat(ncat,a[pos+firstn].name);    //  解析出文件名称
						
						iRead_main(ncat);    //  启动阅读器
						goto beg;
				    }
				}
				break;
				
			case KEY_CTRL_F2:	//  根据输入的文件名打开文件
				memset(subdir_fn, 0, sizeof(subdir_fn));
				if (Subdir_Open(subdir_fn))
				{
					memset(ncat, 0, sizeof(ncat));
				    strcpy(ncat, workdir);
					strcat(ncat, "\\");
				    strcat(ncat, subdir_fn);    //  连接上输入的文件名字
					strcat(ncat, ".txt");
						
					char_to_font(ncat, fname);
					handle = Bfile_OpenFile_OS(fname,0);
					if (handle <= 0)    //  如果文件未找到
					{
						Disp_FileNotFound();
						MsgBoxPop();
						goto beg; break;
					}
						
					MsgBoxPop();
					Bfile_CloseFile_OS(handle);
					
					//  重新绘制浏览器界面
					font16 = open_font("\\\\fls0\\24PX.hzk");
					select_font(font16);
	
					draw_browser(workdir, firstn, pos, a);
					close_font(font16);
					
					//  启动阅读器
					iRead_main(ncat);
				}
				
				goto beg; break;
			
			case KEY_CTRL_EXIT:    //  从文件夹返回根目录
			    if (strcmp(workdir,"\\\\fls0")!=0)    //  如果当前在文件夹内
			    {
			        memset(ncat,0,sizeof(ncat));
			        strncpy(ncat,workdir,strlen(workdir)-strlen(strrchr(workdir,'\\')));
				    memset(workdir,0,sizeof(workdir));
				    strcpy(workdir,ncat);
				    strcat(ncat,"\\*.*");    //  解析出上一级目录的名称
			        a=get_file_list(ncat);    //  浏览该文件夹
					pos=0;firstn=0;    //  初始化列表
				    goto beg;
				
				}
				break;
			
		}
	}
}

void GetDisplayDirName(char *src, char *des)
{
	/*
	    从工作目录中获取所要显示的文件夹层次信息
		参数说明：
			src: 欲处理的工作目录
			des: 输出层次信息存放的缓冲区
	*/
	
	char ncat[64], idm[32];
	char *s;
	
	memset(ncat, 0, sizeof(ncat));
	
	for (s = src + 7; (int)s != 1; s = strchr(s, '\\') + 1)
	{
		strcat(ncat, "\\\\");
		memset(idm, 0, sizeof(idm));
		strncpy(idm, s, strlen(s) - (strchr(s, '\\') ? strlen(strchr(s, '\\')) : 0));
		strcat(ncat, idm);
	}
	
	strcpy(des, ncat);
}

void GetDisplayFileName(char *src, char *des)
{
	/*
	    从工作目录中获取所要显示的文件名称
		参数说明：
			src: 欲处理的工作目录
			des: 输出信息存放的缓冲区
	*/
	
	strcpy(des, strrchr(src, '\\') + 1);
}

int check_consistency()
{
    /*
	    检查文件一致性，若文件有缺失则显示错误信息
	*/

    FONTCHARACTER fname[64];
	int handle,i;
	char_to_font("\\\\fls0\\24PX.hzk",fname);
    handle=Bfile_OpenFile_OS(fname,0);
	if (handle<0) return 0;
	Bfile_CloseFile_OS(handle);
	
	return 1;
}

void main(void) {

    /*
	    程序主函数
	*/

    int key;
    if (!check_consistency())    //  如果文件有缺失，显示错误信息并退出
	{
	    locate_OS(1,1);
		Print_OS("Can't find font file.",0,0);
		while (1) GetKey(&key);
	}
	
	browse_main();    //  否则进入文件浏览器
	return;
}