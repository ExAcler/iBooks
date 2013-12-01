/*

iRead II for Prizm Pro
String library header file

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

char* strcat(char* strDest,const char* strSrc)
{
    char* address=strDest;

    while (*strDest)
      strDest++;
    while(*strDest++=*strSrc++);
    return address;
}

char* strcpy(char* strDest,const char* strSrc)
{
    char* address=strDest;

    while (*strDest++=*strSrc++);
    return address;
}

char* strstr(const char* s1, const char* s2)
{
    int n;

    if (*s2)
    {
        while (*s1)
        {
            for (n=0;*(s1+n)==*(s2+n);n++)
                if (!*(s2+n+1))
                    return (char*)s1;
            s1++;
        }
        return 0;
    }
    else
        return (char*)s1;
}

char* strchr(char* s,char c)
{
    while((*s)&&(*s!=c))
        ++s;
    return (*s==c)?s:0;
}

char* strrchr(const char* str,char ch)
{
    char* p = (char*)str;

    while (*str)
        str++;
    while ((str--!=p)&&(*str!=ch));
    if (*str==ch)
        return (char*)str;
    return 0;
}

int strlen(const char* s)
{
    int i;
	for (i=0;s[i];++i);
	return i;
}

int strcmp(const char* s1,const char* s2)
{
    while((*s1!=0)&&(*s2!=0)&&(*s1==*s2))
	{
        s1++;
        s2++;
    }   
    return *s1-*s2;
}

char* itoa(int val,char* buf,unsigned radix)
{
    char* p;             
    char* firstdig;      
    char temp;           
    unsigned digval;     

    p=buf;

    if (val<0)
    {
        *p++='-';
        val=(unsigned long)(-(long)val);
    }

    firstdig=p;

    do
	{
        digval=(unsigned)(val%radix);
        val/=radix;

        if (digval>9)
            *p++=(char)(digval-10+'a'); 
        else
            *p++=(char)(digval+'0');      
    }
	while (val>0);

    *p--='\0';         

    do
	{
        temp=*p;
        *p=*firstdig;
        *firstdig=temp;
        --p;
        ++firstdig;        
    }
	while(firstdig<p);  

    return buf;
}

int atoi(const char* p)
{
    int neg_flag=0;
    int res=0;
	
    if (p[0]=='+'||p[0]=='-')
        neg_flag=(*p++!='+');
    while (*p>=0x30&&*p<=0x39) res=res*10+(*p++-'0');
    return neg_flag?0-res:res;
}