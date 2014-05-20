#ifndef __LAB5_H__
#define __LAB5_H__

#include "ship.h"
#include "ship2.h"
#include "enemy.h"
#include "enemy2.h"
#include "bullet.h"
#include "box.h"
#include "win.h"
#include "ex1.h"
#include "ex2.h"
#include "ex3.h"
#include "ex4.h"

#define IWRAM (short *)0x03000000
#define REG_DISPCNT (short *)0x04000000
#define MODE_3 0x3
#define BG_2 0x400
#define VRAM (short *)0x06000000
#define REG_KEYINPUT (short *)0x04000130

#define BTN_A 0x01
#define BTN_B 0x02
#define BTN_RT 0x10
#define BTN_LT 0x20
#define BTN_UP 0x40
#define BTN_DN 0x80
#define sWidth 240
#define sHeight 160



void update();
void explode(int x, int y);

#endif
