/*

iRead II for Prizm Pro
Chinese display library source file
Ported by Anderain Lovelace, Copyright (c)2012 All rights reserved.

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

#include <stdio.h>

int Bfile_OpenFile_OS	(const unsigned short*filename, int mode );
int Bfile_CloseFile_OS	(int HANDLE );
int Bfile_ReadFile_OS	(int HANDLE, void *buf, int size, int readpos );

void draw_pic (int x,int y,int width,int height,int sel,unsigned char* pimage);

#include "fxchslib.h"

#define is_chschar(c) ((c) & 0x80)

typedef unsigned short fontc;

FONTFILE * def_font = 0;

static unsigned int x86_dword_to_sh (unsigned int d)
{
	unsigned char c1,c2,c3,c4;
	unsigned int  r;

	c1 = d>>24	;
	c2 = (d & 0xFF0000)		>>16;
	c3 = (d & 0xFF00)		>>8	;
	c4 = (d & 0xFF)				;
	
	r = c1 + (c2<<8) + (c3<<16) + (c4<<24);

	return r;
}

fontc * char_to_font(const char *cfname,fontc *ffname)
{
	int i,len = strlen(cfname);

	for(i=0; i<len ;++i)
		ffname[i] = cfname[i];

	ffname[i]=0;
	return ffname;
}

FONTFILE * open_font(const char * cfname)
{
	int			fh,r;
	FONTFILE	*ff;
	fontc		ffname[32];

	char_to_font(cfname,ffname);

	fh = Bfile_OpenFile_OS(ffname,0);

	if (fh<=0) return NULL;

	ff			 		= (FONTFILE*)malloc(sizeof(FONTFILE));
	Bfile_ReadFile_OS (fh,ff,sizeof(FONTFILE)-2*sizeof(int),0);

	ff->file_handle		= fh;

	ff->width			= x86_dword_to_sh(ff->width);
	ff->height			= x86_dword_to_sh(ff->height);	
	ff->asc_offset		= x86_dword_to_sh(ff->asc_offset);
	ff->chs_offset		= x86_dword_to_sh(ff->chs_offset);

	r					=ff->width/8 + (ff->width % 8 ? 1 : 0);

	ff->font_size		= r*ff->height;

	return ff;
}

void close_font (FONTFILE * ff)
{
	Bfile_CloseFile_OS(ff->file_handle);
	free(ff);
}


void select_font (const FONTFILE * font)
{
	def_font = font;
}

void print_chs_char (int x,int y,int sel,unsigned char c1,unsigned char c2)
{
	unsigned char mat[128];
	int sec,pot;

	if (def_font==NULL) return;

 
	sec = c1-0xa1;
	pot = c2-0xa1;

	Bfile_ReadFile_OS( def_font->file_handle,
					mat,
					def_font->font_size,
					(94*sec+pot)*def_font->font_size + def_font->chs_offset);

	draw_pic(x,y,def_font->width,def_font->height,sel,mat);
}

void print_asc_char (int x,int y,int sel,unsigned char c)
{
	unsigned char mat[128];

	if (def_font==NULL) return;


	Bfile_ReadFile_OS( def_font->file_handle,
					mat,
					def_font->font_size,
					c*def_font->font_size + def_font->asc_offset);

	draw_pic(x,y,def_font->width,def_font->height,sel,mat);
}

void print_chs_str (int x,int y,int sel,const unsigned char * str)
{
	int		i;
	int		l = 0;
	int		cx;

	if (! def_font) return ;

	for (i=0;str[i];l++)
	{
		cx = x+l*def_font->width;

		if (is_chschar(str[i]))
		{
			
			print_chs_char (cx,y,sel,str[i],str[i+1]);
			++i,++i;
		}
		else
		{
			++i;
			if (! def_font->asc_offset) continue;
			print_asc_char (cx,y,sel,str[i-1]);
		}
		
	}
}

