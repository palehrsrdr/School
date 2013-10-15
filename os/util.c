//util.c
#include "util.h"
void kmemcpy(void *d, void *s, int c)
{
	int i;
	char *dp = (char*) d;
	char *sp = (char*) s;
	for (i = 0; i < c; i++, sp++, dp++)
		*dp = *sp;
}
void halt()
{
	asm volatile("hlt");
}