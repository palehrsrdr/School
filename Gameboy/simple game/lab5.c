#include "lab5.h"

// defined in linker script
extern int ship_image;
extern int image_size;
int EActive = 1;
int BActive = 0;
int RIGHT = 1;
int WIN = 0;



int shipx, shipy, bx,by,enx,eny,bspd;
int main()
{
	// setup video

	*REG_DISPCNT = MODE_3 | BG_2;
	
	// get address of ship image (& keeps it from dereferencing)
	char *src = (char *)&ship_image;
	char *dst = (char *)IWRAM;
	int i = (int)&image_size;
	int z = 0;

	enx =0;
	eny = 0;
	shipx = 80;
	shipy = 128;
	bx = shipx+12;
	by = shipy-bullet[0];
	bspd = 4;

	// copy ship image into IWRAM
	while (i > 0)
	{
		*dst = *src;
		++dst;
		++src;
		--i;
	}

	while(1)
	{
		update();
	}
}

void draw(int xpos, int ypos, int tag)
{
	// offsets for image locations in memory
	int offs;
	if (tag == 0) offs = 0;									
	if (tag == 1) offs = (ship[0] * ship[1]) + 2; 		 	
	if (tag == 2) offs = (ship[0] * ship[1])+ (enemy[0] * enemy[1])+ 4; 
	if (tag == 3) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + 6;
	if (tag == 4) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + 8;
	if (tag == 5) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + (bullet[0] * bullet[1]) + 10;
 	if (tag == 6) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + (bullet[0] * bullet[1]) + (box[0] * box[1]) +12;
 	
 	if (tag == 7) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + (bullet[0] * bullet[1]) + (box[0] * box[1]) + (win[0] * win[1])+14;
 	if (tag == 8) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + (bullet[0] * bullet[1]) + (box[0] * box[1]) + (win[0] * win[1])+ 1024+16;
 	if (tag == 9) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + (bullet[0] * bullet[1]) + (box[0] * box[1]) + (win[0] * win[1])+2048+ 18;
 	if (tag == 10) offs = (ship[0] * ship[1])+ (ship2[0] * ship2[1])+ (enemy[0] * enemy[1]) + (enemy2[0] * enemy2[1]) + (bullet[0] * bullet[1]) + (box[0] * box[1]) + (win[0] * win[1])+3072 + 20;
	 	
 	int x,y;
	short *vram = VRAM;
	short *iwram = IWRAM + offs;
	short width = *iwram++;
	short height = *iwram++;

	vram += xpos;
	vram += ypos * sWidth;

	for (y = 0; y < height; ++y)
	{
		for (x = 0; x <width; x++)
			*vram++ = *iwram++;
		vram += sWidth-width;
	}
}

void update()
{
	if(!(*REG_KEYINPUT & BTN_RT) && shipx < sWidth-ship[0] && !WIN) 
		shipx += 1;
	
	if (!(*REG_KEYINPUT & BTN_LT) && shipx > 0 && !WIN )
		shipx -= 1; 

	if(!(*REG_KEYINPUT & BTN_A) && !BActive && !WIN)
	{
		BActive = 1;
		bx = shipx+12;
	}
	
	if (EActive)
	{
		if(enx > sWidth - enemy[0]) RIGHT = 0;
		if(enx < 0) RIGHT = 1;		
		if(RIGHT)enx += 1;
		else enx -=1;
	}
	if (BActive)
	{
		draw(bx,by,4);
		by -= bspd;
		// hit detection for enemy
		if (by < -8)
		{
			by =shipy-bullet[0];
			BActive = 0;
		}
		if (by < eny+enemy[0]-bullet[0])
		{
			if (bx < enx + enemy[0] && bx > enx)
			{
				EActive = 0;
				BActive = 0;
				draw(bx,by-bspd,5);
				bx = shipx+12;
				by = shipy-bullet[0];
				draw((sWidth-win[0])/2,80-win[1],6);
				explode(enx, eny);
				WIN = 1;				
			}
			
		}
	}
	if (!WIN)
	{
		draw(shipx,shipy,0);
		draw(shipx,shipy,1);
		draw(enx,eny,2);
		draw(enx,eny,3);
		draw(bx,by+bspd,5);
	}
	
}

void explode(int x, int y)
{
	int z;
	draw(x,y,7);
	for(z=0; z<10000; z++);
	draw(x,y,8);
	for(z=0; z<10000; z++);
	draw(x,y,9);
	for(z=0; z<10000; z++);
	draw(x,y,10);
}
