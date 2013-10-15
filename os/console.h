//console.h
#pragma once
enum color {
BLACK, NAVY, GREEN, CYAN, RED,
PURPLE, OLIVE, GREY, BLACK2,
BLUE, LIME, TURQUOISE, PINK,
MAGENTA, YELLOW, WHITE } ;

void cosnole_init();
void console_clear();
void console_clrline(int line);
int console_set_color(enum color fg, enum color bg);
void scroll();
