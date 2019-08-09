/*
 * EX1.c
 *CDS의 밝기에 따라 다단게로 LED 밝기는 조절되고, pc에서 입력에 따라 p456가 동작되는 코딩
 * Created: 2019-07-05 오후 4:10:04
 *  Author: 301-01
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define  DIR_L 0
#define  DIR_R 1

unsigned char timer0Cnt=0,  mot_cnt=0;
volatile unsigned char dir = DIR_R;
volatile unsigned char Step_flag = 0, buzzer_flag = 0, LCD_flag = 0;

unsigned char Step[] = {0x90, 0x80, 0xC0, 0x40, 0x60, 0x20, 0x30, 0x10};
	
unsigned int DoReMi[8] = {523,587,659,698,783,880,987,1046};


volatile unsigned int ADC_result = 0;

void putch(unsigned char data)
{
	while((UCSR0A & 0x20)==0);
	UDR0 = data;
	UCSR0A |= 0x20;
}

void putch_Str(char *str)
{
	unsigned char i=0;
	while(str[i]!='\0')
	putch(str[i++]);
}




int main(void)
{
	
	unsigned int AdData = 0;
	
	DDRF = 0x00;
	DDRE = 0x02;
	
	UCSR0A =0x00;
	UCSR0B = 0x18;
	UCSR0C = 0x06;
	
	UBRR0H = 0x00;
	UBRR0L = 0x03;
	
	ADMUX = 0x40;
	ADCSRA = 0xAF;
	ADCSRA |= 0x40;
	
	sei();
	while(1)
	{
		AdData = ADC_result;
		
		putch_Str("\n\r CDS ADC_data : ");
		putch((AdData/1000)%10+'0');
		putch((AdData/100)%10+'0');
		putch((AdData/10)%10+'0');
		putch((AdData)%10+'0');
		_delay_ms(500);
	}
	
	
	unsigned char piano = 0;
	
	DDRB = 0xA0;
	DDRD = 0xF0;
	DDRE = 0x00;
	
	PORTB & = ~0x20;
	
	LcdInit_4bit();
	
	TCCR0 = 0x03;
	TCNT0 = 184;
	TCCR1A |= 0x0A;
	TCCR1B |= 0x19;
	TCCR1C = 0x00;
	TCNT1 = 0x0000;
	
	TIMSK = 0x01;
	TIFR = 0x01;
	
	EICRB = 0xFF;
	EIMSK = 0xF0;
	EIFR = 0xF0;
	sei();
	
	Lcd_Pos(0,0);
	Lcd_CHAR("Buzzer : OFF",12);
	
	while(1)
	{
		if(LCD_flag)
		{
			if(LCD_flag & 0x01)
			{
				Lcd_Pos(13,0);
				Lcd_CHAR("OFF",3);
				Lcd_Pos(11,1);
				Lcd_CHAR("OFF",3);
				LCD_flag &= 0x0E;
			}
			if(LCD_flag & 0x02)
			{
				Lcd_Pos(11,1);
				Lcd_CHAR("ON",3);
				
				buzzer_flag = 1;
				LCD_flag & = 0x0D
			}
			if(LCD_flag & 0x04)
			{
				Lcd_Pos(13,0);
				Lcd_CHAR("CW",3);
				LCD_flag & 0x0B;
			}
			if(LCD_flag & 0x08)
			{
				Lcd_Pos(13,0);
				Lcd_CHAR("CCW",3);
				LCD_flag & = 0x07;
			}
			if(buzzer_flag)
			{
				ICR1 = 14745600/DoReMi[piano];
				
				OCR1C = ICR1/2;
				piano++;
				if(8 < piano) piano =0;
				_delay_ms(1000);
			}
		}
	}
}

SIGNAL(ADC_vect)
{
	cli();
	ADC_result = ADC;
	sei();
}


SIGNAL(INT4_vect)
{
	cli();
	Step_flag = 0;
	PORTD = 0;
	buzzer_flag = 0;
	OCR1C = 0;
	LCD_flag |= 0x01;
	sei();
}
SIGNAL(INT5_vect)
{
	cli();
	LCD_flag |= 0x02;
	sei();
}

SIGNAL(INT6_vect)
{
	cli();
	Step_flag = 1;
	dir = DIR_R;
	LCD_flag |= 0x04;
	sei();
}

SIGNAL(INT7_vect)
{
	cli();
	
	Step_flag = 1;
	dir = DIR_L;
	LCD_flag |= 0x08;
	sei();
}