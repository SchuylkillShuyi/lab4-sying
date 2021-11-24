/*
 * GccApplication4.c
 *
 * Created: 11/5/2021 3:10:28 PM
 * Author : ysy10
 */ 

#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)
#define R 4
#define L 15
#define X_CENTER 79
#define Y_CENTER 63


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

struct Speed
{
	int x;
	int y;
};
struct Speed ball_speed={4,0};

void Initialize()
{
	/*---------------------setup for ADC--------------------*/
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
	
	/*--------------------wireless control----------------*/
	//PD4 is the input for computer button
	DDRD &= ~(1<<DDD4);
	
	
	/*---------------------Set up LED----------------------*/
	//PD2 is output for yellow LED (user win)
	DDRD |= (1<<DDD2); 
	
	//PD3 is output for red LED (computer win)
	DDRD |= (1<<DDD3); 
	
	/*---------------------Set up buzzer--------------------*/
	//PD5 is output for buzzer
	DDRD |= (1<<DDD5); 
	
	//timer0 for phase correct	
	//set timer0 to be divided by 1024, which is 15625 Hz
	TCCR0B |= (1<<CS00); 
	TCCR0B &= ~(1<<CS01); 
	TCCR0B |= (1<<CS02);
	
	//set timer0 to PWM phase correct mode
	TCCR0A |= (1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B |= (1<<WGM02);
	
	// clear OC0B on compare match
	TCCR0A &= ~(1<<COM0B0);
	TCCR0A |= (1<<COM0B1);
	
	// clear interrupt flag
	TIFR0 |= (1<<OCF0A);
	
	// mute at the beginning
	OCR0A = 0; 
	
	//set duty cycle as 5%
	OCR0B = OCR0A* 0.95;

	/*-------------------Set up LCD------------------------*/
	lcd_init();
	
	sei();
}


ISR (ADC_vect)
{
	//sprintf(String,"ADC : %d \n", ADC);
	//UART_putstring(String);
}

struct Speed set_speed(struct Speed s)
{
	s.x=rand()%7-3;
	s.y=rand()%7-3;
	if(s.x==0)
	{
		s.x+=1;
	}
	if(s.y==0)
	{
		s.y+=1;
	}
	return s;
}



int Game_Modes(uint8_t user, uint8_t computer, struct Speed s)
{	
	//initialize ball
	
	int8_t ball_speed_x = set_speed(ball_speed).x;
	int8_t ball_speed_y = set_speed(ball_speed).y;
	uint8_t ball_x = X_CENTER;
	uint8_t ball_y = Y_CENTER;
	
	//initialize computer paddle
	int8_t computer_speed = 5;
	uint8_t computer_y0 = Y_CENTER-L;
	uint8_t computer_y1 = Y_CENTER+L; 
	
	//initialize user paddle
	uint8_t user_speed = 0;
	uint8_t user_y0 = Y_CENTER-L;
	uint8_t user_y1 =Y_CENTER+L; 
	
	
	/*----------------------score board------------------------*/
	
	LCD_drawChar( 10 , 10 , user , CYAN, BLACK);
	LCD_drawChar( 150 , 10 , computer, CYAN, BLACK);
	

	while (1)
	{	
		
		/*------------------------user paddle function------------------------*/
		
		LCD_drawBlock(0, user_y0, 3, user_y1, BLACK); // invisible
		
		if(ADC <520) //go up
		{
			if(user_y1 <= 122 )// touch the upper bound and bounce
			{
				user_speed = 5;
			}
			else
			{
				user_speed = 0;
			}
		}
		else if(ADC > 560) //go down
		{	
			if(user_y0 >= 5) //touch the lower bound and bounce
			{
				user_speed = -5;
			}
			else
			{
				user_speed = 0;
			}
		}
		else if (ADC >= 520 && ADC <= 560) // stand still
		{
			user_speed = 0;
			
		}
		
		user_y0 = user_y0 + user_speed;
		user_y1 = user_y1 + user_speed;
		LCD_drawBlock(0, user_y0, 3, user_y1, YELLOW); 
		

		/*---------------------computer paddle function--------------------*/
		
		LCD_drawBlock(156, computer_y0, 159, computer_y1, BLACK); //invisible
		
		//up button
		
		if(PIND&(1<<PIND4))
		{ 
			
			if(computer_y1> 122)
			{
				
				computer_speed = 0;
				
			}
			else
			{
				computer_speed =5;
			}
		}
		else
		{
			if(computer_y0 <2 )
			{
				computer_speed = 0;
			}
			else
			{
				computer_speed=-5;
			}
		}
		
		
		computer_y0 = computer_y0 + computer_speed;
		computer_y1 = computer_y1 + computer_speed;
		LCD_drawBlock(156, computer_y0, 159, computer_y1, RED);

		/*---------------------circle move function-----------------------*/
		
		LCD_drawCircle(ball_x, ball_y, R, BLACK); //invisible
		
		if(ball_y <= 2 ) // up boundary detection 
		{	
			OCR0A = 3;  // set the top value as 16*10^6/(2*2.5k*1024)=3
			OCR0B = OCR0A*0.95;
			
			_delay_ms(500);
			
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A * 0.95;
			
			ball_speed_y = -1 * ball_speed_y; // go back
		}
		
		else if(ball_y >=125) //bottom boundary detection 
		{	
			OCR0A = 3;  // set the top value as 16*10^6/(2*2.5k*1024)=3
			OCR0B = OCR0A* 0.95;
			
			_delay_ms(500);
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A * 0.95;
			
			ball_speed_y = -1 * ball_speed_y; // go back
		}
		
		else if(ball_x <= 3) // left boundary detection
		{
			ball_speed_x = -1 * ball_speed_x;
			OCR0A = 3;  // set the top value as 16*10^6/(2*2.5k*1024)=3
			OCR0B = OCR0A * 0.95;
			
			_delay_ms(500);
			
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A * 0.95;
			
			// user wins
			if(ball_y < user_y0 || ball_y > user_y1) 
			{	
				LCD_drawChar( 10 , 10 , user, BLACK, BLACK);
				LCD_drawChar( 150 , 10 , computer, BLACK, BLACK);
				
				PORTD |= (1<<PORTD3); //turn on the yellow LED
				_delay_ms(500);
				PORTD &= ~(1<<PORTD3); //turn off the yellow LED
				
				return 0; 
			}
		}
		
		else if(ball_x >= 157)// right boundary detection 
		{
			ball_speed_x = -1 * ball_speed_x;
			
			OCR0A = 3;  // set the top value as 16*10^6/(2*2.5k*1024)=3
			OCR0B = OCR0A * 0.95;
			
			_delay_ms(500);
			
			OCR0A = 0; //stop the buzzer
			OCR0B = OCR0A * 0.95;
			
			if (ball_y < computer_y0 || ball_y > computer_y1 ) // computer wins
			{	
				LCD_drawChar( 10 , 10 , user, BLACK, BLACK);
				LCD_drawChar( 150 , 10 , computer, BLACK, BLACK);
				
				PORTD |= (1<<PORTD2); // turn on the red LED
				_delay_ms(500);
				PORTD &= ~(1<<PORTD2); // turn off the red LED
				
				return 1; 
			}
		}
		
		ball_x = ball_x + ball_speed_x; 
		ball_y = ball_y + ball_speed_y;
		
		// ball fly out of boundary condition
		
		if(ball_x< 0) 
		{
			ball_x = 2;
		}
		else if (ball_x > 159)
		{
			ball_x = 157;
		}
		if(ball_y < 0)
		{
			ball_y = 2;
		}
		else if (ball_y > 127)
		{
			ball_y =125;
		}
		
		LCD_drawCircle(ball_x, ball_y, R, WHITE);
		
	}
}

int main(void)
{	
	int win;
	uint8_t user = 0x30; 
	uint8_t computer = 0x30;
	
	Initialize();
	
	//set screen as black
	LCD_setScreen(BLACK);
	
	LCD_drawString(X_CENTER-40, Y_CENTER, "Game Start", CYAN, BLACK);
	_delay_ms(1000);
	LCD_drawString(X_CENTER-40, Y_CENTER, "Game Start", BLACK, BLACK);
	//int a=PIND&(1<<PIND4);
	sprintf(String,"111");
	UART_putstring(String);
	// Ball with R=4 start to move at the center
	LCD_drawCircle(X_CENTER, Y_CENTER, R, WHITE);
	
	for(int i = 0; i < 5; i++)
	{	
		if(user < 0x32 && computer< 0x32)
		{ 
			win = Game_Modes(user, computer, ball_speed);
			if(win == 0) // computer wins
			{
				computer += 1;
			}
			else //user wins
			{
				user += 1;
			}
			LCD_setScreen(BLACK);
		}
		else if (user >=0x32 )
		{
			LCD_drawString(X_CENTER-40, Y_CENTER, "User Win", YELLOW, BLACK);
		}
		else if(computer >= 0x32)
		{
			LCD_drawString(X_CENTER-40, Y_CENTER, "Computer Win", RED, BLACK);
		}
	}
	
	 
}
