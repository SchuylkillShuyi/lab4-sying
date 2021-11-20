
/*
 * GccApplication4.c
 *
 * Created: 11/5/2021 3:10:28 PM
 * Author : ysy10
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

#include <avr/io.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include <time.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>

char String[25];

void Initialize()
{
	/*-------------------Set up LCD------------------------*/
	lcd_init();
	
	/*---------------------Set up LED----------------------*/
	DDRD |= (1<<DDD2); //set PD2 as output for player 1 LED
	DDRD |= (1<<DDD3); //set PD3 as output for player 2 LED
	
	/*---------------------Set up buzzer--------------------*/
	DDRD |= (1<<DDD5); //set pd5 as output
	
	/*----------------------set timer0------------------*/
		
	TCCR0B |= (1<<CS00); //Set the prescaler
	TCCR0B &= ~(1<<CS01); //scale with 1024
	TCCR0B |= (1<<CS02);
	
	//pwm phase correct mode
	TCCR0A |= (1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B |= (1<<WGM02);
	OCR0A = 0;  // set the top value as 16*10^6/(2*440*1024)=18 but I want it to be quite right now
	OCR0B = OCR0A/2; //set the OCRnB as 50% duty cycle
	TCCR0A |= (1<<COM0B1); // clear OC0B on compare match
	TCCR0A &= ~(1<<COM0B0); // clear OC0B on compare match
	
	/*---------------Set up timer 1-----------------*/
	cli();
	//timer1 is for LED and buzzer
	//Timer	1 setup prescaller with 1024
	TCCR1B |= (1<<CS12);
	TCCR1B &= ~(1<<CS11);
	TCCR1B &= ~(1<<CS10);
	
	//set timer 1 to normal
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	
	/*-----------------setup for ADC------------------*/
	// clear power reduction for ADC
	PRR &= ~(1<<PRADC);
	
	//select Vref = AVcc
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	
	//set the ADC Clock div by 128
	//16M/128=125kHz
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	//select channel 0
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	//Set to auto trigger
	ADCSRA |= (1<<ADATE);
	
	//set to free running
	ADCSRB &= ~(1<<ADTS0);
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);
	//Disable digital input buffer on ADC pin
	DIDR0 |= (1<<ADC0D);
	// Enable ADC
	ADCSRA |= (1<<ADEN);
	//Enable ADC Interrupt
	ADCSRA |= (1<<ADIE);
	//Start Conversion
	ADCSRA |= (1<<ADSC);
	
	
	sei();
	
	
	/*----------------------set screen-------------*/
	LCD_setScreen(BLACK); //set screen as black
	LCD_drawCircle(79, 63, 4, GREEN); // initialize ball
	LCD_drawBlock(156, 59, 159, 67, RED); //initialize computer block
}

ISR(TIMER1_OVF_vect)
{
	
}
ISR (ADC_vect)
{
	//sprintf(String,"ADC : %d \n", ADC);
	//UART_putstring(String);
}

int GameStart(uint8_t left_win, uint8_t right_win)
{	
	uint8_t x_speed = rand()%7 - 3 ; //random initialize a horizon speed between -5 to 5
	uint8_t y_speed = rand()%7 - 3; //random initialize a horizon speed between -5 to 5
	uint8_t x = 79;
	uint8_t y = 63;
	uint8_t block_y_speed = 5;
	//uint8_t block_x0 = 156;
	uint8_t block_y0 = 43;
	//uint8_t block_X1 = 159;
	uint8_t block_y1 = 83; // define computer controlled block
	uint8_t user_y0 = 53;
	uint8_t user_y1 =73; // define user controlled block l
	uint8_t user_y_speed = 0;
	
	
	//LCD_drawLine(10, 10, 50, 50, RED);
	if(x_speed == 0)
	{
		x_speed = x_speed + 1;
	}
	if(y_speed == 0)
	{
		y_speed = y_speed + 1;
	}
	
	LCD_drawChar( 9 , 9 , left_win , MAGENTA, BLACK);
	LCD_drawChar( 149 , 9 , right_win, MAGENTA, BLACK);
	while (1)
	{	
		
		// user controlled block function 
		LCD_drawBlock(0, user_y0, 3, user_y1, BLACK);
		if(ADC <400)
		{
			if(user_y1 <= 122 )
			{
				user_y_speed = 5;
			}
			else
			{
				user_y_speed = 0;
			}
			
			
		}
		else if(ADC > 750)
		{	
			if(user_y0 >= 5)
			{
				user_y_speed = -5;
			
			}
			else
			{
				user_y_speed = 0;
				
			}
			
		}
		else if (ADC >= 400 && ADC <= 750)
		{
			user_y_speed = 0;
			
		}
		user_y0 = user_y0 + user_y_speed;
		user_y1 = user_y1 + user_y_speed;
		LCD_drawBlock(0, user_y0, 3, user_y1, RED);
		
		////////////////////////////////////////////////////////// 
		//computer block move function
		LCD_drawBlock(156, block_y0, 159, block_y1, BLACK);
		if(block_y0 <6)
		{
			block_y_speed = 5;
			//block_y0 = 0;
			//block_y1 = 19;
		}
		else if(block_y1 > 118)
		{
			block_y_speed = -5;
			//block_y0 = 108;
			//block_y1 = 127;
		}
		block_y0 = block_y0 + block_y_speed;
		block_y1 = block_y1 + block_y_speed;
		LCD_drawBlock(156, block_y0, 159, block_y1, RED);
		////////////////////////////////////////
		//circle move function
		LCD_drawCircle(x, y, 4, BLACK);
		if(y <= 2 ) // up boundary detection
		{	OCR0A = 18;  // set the top value as 16*10^6/(2*440*1024)=18 but I want it to be quite right now
			OCR0B = OCR0A/2;
			_delay_ms(300);
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A/2;
			y_speed = -1 * y_speed;
		}
		else if(y >=125) //bottom boundary detection
		{	
			OCR0A = 18;  // set the top value as 16*10^6/(2*440*1024)=18 but I want it to be quite right now
			OCR0B = OCR0A/2;
			_delay_ms(300);
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A/2;
			y_speed = -1 * y_speed;
		}
		else if(x <= 3) // left boundary detection and round detection
		{
			x_speed = -1 * x_speed;
			OCR0A = 18;  // set the top value as 16*10^6/(2*440*1024)=18 but I want it to be quite right now
			OCR0B = OCR0A/2;
			_delay_ms(300);
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A/2;
			if(y < user_y0 || y > user_y1)
			{	
				LCD_drawChar( 9 , 9 , left_win, BLACK, BLACK);
				LCD_drawChar( 149 , 9 , right_win, BLACK, BLACK);
				PORTD |= (1<<PORTD3);
				_delay_ms(1000);
				PORTD &= ~(1<<PORTD3);
				//*right_win = *right_win + 1;
				return 0; //right wins return 0
				//break;
			}
		}
		else if(x >= 157)// right boundary detection and round detection
		{
			x_speed = -1 * x_speed;
			OCR0A = 18;  // set the top value as 16*10^6/(2*440*1024)=18 but I want it to be quite right now
			OCR0B = OCR0A/2;
			_delay_ms(300);
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A/2;
			if (y < block_y0 || y > block_y1 )
			{	
				LCD_drawChar( 9 , 9 , left_win, BLACK, BLACK);
				LCD_drawChar( 149 , 9 , right_win, BLACK, BLACK);
				PORTD |= (1<<PORTD2);
				_delay_ms(1000);
				PORTD &= ~(1<<PORTD2);
				return 1; //left wins return 1
				//break;
			}
		}
		x = x + x_speed; //relocate x and y
		y = y + y_speed;
		if(x< 0) // ball fly out of boundary condition
		{
			x = 2;
		}
		else if (x > 159)
		{
			x = 157;
		}
		if(y < 0)
		{
			y = 2;
		}
		else if (y > 127)
		{
			y =125;
		}
		LCD_drawCircle(x, y, 4, GREEN);
		/////////////////////////////////////////////////
	}
}
int main(void)
{	
	Initialize();
	int result;
	uint8_t left_win = 48; // decimal value to hex value in LUT
	uint8_t right_win = 48;
	for(int i = 0; i < 12; i++)
	{	if(left_win < 50 && right_win < 50)
		{ 
			result = GameStart(left_win, right_win);
			if(result == 0)
			{
				right_win = right_win + 1;
			}
			else
			{
				left_win = left_win + 1;
			}
			LCD_setScreen(BLACK);
		}
		else if (left_win >=50 )
		{
			LCD_drawString(79, 63, "Left Player Win", MAGENTA, BLACK);
			break;
		}
		else if(right_win >= 50)
		{
			LCD_drawString(79, 63, "Right Player Win", MAGENTA, BLACK);
			break;
		}
	}
	
	 
}
