/*

iRead II for Prizm Pro
Browser library header file

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

#define DT_DIRECTORY 0

typedef unsigned short FONTCHARACTER;

typedef struct
{
    unsigned short  id;
    unsigned short  type;
    unsigned long   fsize;                  // File size
    unsigned long   dsize;                  // Data size
    unsigned int    property;               // The file has not been completed, except when property is 0.
    unsigned long   address;
} FILE_INFO;

int Bfile_FindClose( int FindHandle );
int Bfile_FindFirst( const FONTCHARACTER *pathname, int *FindHandle, FONTCHARACTER *foundfile, FILE_INFO *fileinfo );
int Bfile_FindNext( int FindHandle, FONTCHARACTER *foundfile, FILE_INFO *fileinfo );

char** get_file_list(const char* path);
void draw_browser(const char* path,int firstf,int selp,char** a);