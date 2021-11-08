/*
 * GccApplication4.c
 *
 * Created: 11/5/2021 3:10:28 PM
 * Author : ysy10
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include <math.h>

void Initialize()
{
	lcd_init();
}

int main(void)
{
	Initialize();
	//LCD_drawCircle(80, 64, 10, BLACK);
	//LCD_drawLine(50,50,100, 30, MAGENTA);
	//LCD_drawBlock(1,1,20,20,YELLOW);
	LCD_setScreen(BLACK);
	LCD_drawString(80,64,"YSYNB",MAGENTA,BLACK);
    while (1) 
    {
		 
    }
}
