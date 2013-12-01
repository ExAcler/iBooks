/*

iRead II for Prizm Pro
Chinese display library header file
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

typedef struct
{
	char			author[16]		;
	unsigned int	width,height	;
	unsigned int	asc_offset		;
	unsigned int	chs_offset		;
	int				file_handle		;
	int				font_size		;
}
FONTFILE;


FONTFILE *	open_font(const char * cfname);
void		close_font (FONTFILE * ff);
void		select_font (const FONTFILE * font);
void		print_chs_char (int x,int y,int sel,unsigned char c1,unsigned char c2);
void		print_asc_char (int x,int y,int sel,unsigned char c);
void		print_chs_str (int x,int y,int sel,const unsigned char * str);




