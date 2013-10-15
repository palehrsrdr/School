// UTIL.H
#pragma once
#define outb(v,p)\
	asm volatile(\
	"outb %%al, %%dx"\
	: : "a"(v), "d"(p))
#
void kmemcpy(void *d, void *s, int c);
void halt();