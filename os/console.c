#include "console.h"
#include "util.h"

int CC =0;
int CR =0;
int curColor = RED;
void scroll()
{
	int num = 160*24;
	int i;
	void* src = (void*)(0xb8000+160);
	void* dest = (void*)(0xb8000);
	kmemcpy(dest,src,num);

	CC = 0;
	CR = 24;
	char* curs = (char*)0xb8000+(24*160+CC*2);
	for(i=0;i<80*2;i++)
	{
		curs[i] = ' ';
		i++;
		curs[i] = console_set_color(RED,WHITE);
	}
	
	CC = 0;
	CR = 24;
}
void console_putc(char x)
{
	if(x == '\n')
	{
		CC = 0;
		CR++;
	}
	else if(x == '\t')
	{
		int spaces = (CC & 7);
		while (spaces < 8)
		{
			console_putc(' ');
			spaces++;
		}

	}
	else
	{
		char * curs = (char *) 0xb8000 + (CR*160) + (CC *2);
		*curs = x;
		curs++;
		*curs = console_set_color(RED,NAVY);
 		CC++;
	}
	
	
 	if(CC > 79)
	{
		CR++;
		CC = 0;
		
	}
	if (CR > 24)
	{
		scroll();
	}
}
void console_clear()
{
	int i = 0;
	while(i < 80*25)
	{
		console_putc(' ');
		i++;
	}
}
void console_clrline(int line)
{
	char * curs = (char *) 0xb8000 + (line*160);
	int i = 0;
	while(i<80)
	{
		curs[i] = 'P';
		i++;		
	}

}
int console_set_color(enum color fg, enum color bg)
{
	int f = (int) fg;
	int b = (int) bg;
	int x = (b << 4) | f;
	return x;
}
void console_init()
{
	console_clear();
}

