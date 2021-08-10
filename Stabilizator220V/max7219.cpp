/* 
* MAX7219.cpp
*
* Created: 18.04.2018 11:23:22
* Author: Andrey
*/


#include "max7219.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//----------------------
#define A   0x40
#define B   0x20
#define C   0x10
#define D   0x08
#define E   0x04
#define F   0x02
#define G   0x01
#define H   0x80
//--------------------------
const unsigned int ZnGen[]PROGMEM=
{
	A+B+C+D+E+F,				//0x00 - 0
	B+C,					//0x01 - 1
	A+B+G+E+D,				//0x02 - 2
	A+B+C+D+G,				//0x03 - 3
	B+C+F+G,				//0x04 - 4
	A+C+D+F+G,				//0x05 - 5
	A+C+D+E+F+G,				//0x06 - 6
	A+B+C,					//0x07 - 7
	A+B+C+D+E+F+G,				//0x08 - 8
	A+B+C+D+F+G,				//0x09 - 9
	A+B+C+E+F+G,				//0x0A - A
	C+D+E+F+G,				//0x0B - b
	A+D+E+F,				//0x0C - C
	B+C+D+E+G,				//0x0D - d
	A+D+E+F+G,				//0X0E - E
	A+E+F+G,				//0x0F - F
	G,					//0x10 - -
	0x00,					//0x11 - пробел
	A+C+D+E+F,				//0x12 - G
	E+G,					//0x13 - r
	C+E+G,					//0x14 - n
	C+E+F+G,				//0x15 - h
	B+C+E+F+G,				//0x16 - H
	C+D+E+G,				//0x17 - o
	A+B+E+F+G,				//0x18 - P
	A+B+C+E+F,				//0x19 - П
	A+E+F,					//0x1A - Г
	D+E+F+G,				//0x1b - t
	B+C+D+E+F,				//0x1c - U
	C+D+E,					//0x1d - u
	D+E+F					//0x1e - L
	
};



// default constructor
MAX7219::MAX7219()
{
	//init();
} //MAX7219

// default destructor
MAX7219::~MAX7219()
{
} //~MAX7219

void MAX7219::init(void)
{
	STB_DDR |=(1<<STB_PIN);
	STB_PORT|=(1<<STB_PIN);
	CLK_DDR |=(1<<CLC_PIN);
	CLK_PORT&=~(1<<CLC_PIN);
	DIO_DDR |= (1<<DIO_PIN);

	send_cmd(0x09,0x00);//Decode Mode Режим декодирования нет
//	send_cmd(0x0A,0x0F);//Intensity (Яркость наверное Максимальная)
	set_brightness(6); // яркость (0...15)
	send_cmd(0x0b,0x07); //Scan Limit Сканируем 7 цифр
	send_cmd(0x0c,0x01); //Shutdown (Нормальный режим)
	send_cmd(0x0f,0x00); //Display Test  (Нормальный режим)
	clear();
	display();
}
//-------------------------------------------------------
//void MAX7219::send(uint8_t val)
//{
	//uint8_t z;
////	DIO_DDR |= (1<<DIO_PIN);
	//for(z=0;z<8;z++)
	//{
		//uint8_t sreg = SREG;
		//cli();
		//CLK_PORT &=~(1<<CLC_PIN);
		//if(val & 0x80) DIO_PORT|=(1<<DIO_PIN); else DIO_PORT&=~(1<<DIO_PIN);
		//CLK_PORT |=(1<<CLC_PIN);
		//val <<=1;
		//SREG = sreg;
	//}
	//CLK_PORT &=~(1<<CLC_PIN);
//}

//-----------------------------------------
void MAX7219::send(uint16_t val)
{
	uint8_t z;
//	DIO_DDR |= (1<<DIO_PIN);
	for(z=0;z<16;z++)
	{
//		uint8_t sreg = SREG;
//		cli();
		CLK_PORT &=~(1<<CLC_PIN);
		if(val & 0x8000) DIO_PORT|=(1<<DIO_PIN); 
			else DIO_PORT&=~(1<<DIO_PIN);
		CLK_PORT |=(1<<CLC_PIN);
		val <<=1;
//		SREG = sreg;
	}
	CLK_PORT &=~(1<<CLC_PIN);	
}
//-----------------------------------------
void MAX7219::set_brightness(uint8_t br)//установить яркость 0...15
{
	brightness = br & 0x0F;
	send_cmd(0x0A , brightness);
}
//----------------------------------------------------------
void MAX7219::send_cmd(uint8_t reg,uint8_t val)
{
STB_PORT &=~(1<<STB_PIN);

uint8_t sreg = SREG;
cli();
send((uint16_t)((uint16_t)reg << 8 | val));
SREG = sreg;

//send(reg);
//send(val);
STB_PORT |=(1<<STB_PIN);
}

//------------------------------------------
void MAX7219::set_cur(uint8_t cur)
{
	if(cur > 7)cur = 7;
	poz = cur;
}
//------------------------------------------
void MAX7219::clear(uint8_t n , uint8_t z )
{
	poz=0;
	if((n+z)>8)return;
	while(z--)
	{
		led_buf[n++] = 0x00;
	}
}
//---------------------------------------------------------
void MAX7219::display(void)
{

uint8_t z;
uint16_t d;
	for(z=0;z<8;z++)
	{
		d = led_buf[z];
		if((d & 0x200)&&(flash_mark == true)) send_cmd(8-z,(uint8_t)(d & 0x0000));else
		send_cmd(8-z,(uint8_t)(d & 0x00ff));
	}

}
//-----------------------------------------
void MAX7219::set_flash(uint8_t mask)//мигать по маске
{
	uint8_t z;
	for(z=0;z<8;z++)
	{
		if(mask & 0x01)led_buf[z] |= 0x200;
		mask >>=1;
	}
}
//----------------------------------------------------------
void MAX7219::clr_flash(uint8_t mask)//выкл. мигание по маске
{
	uint8_t z;
	for(z=0;z<8;z++)
	{
		if(mask & 0x01)led_buf[z] &= ~(0x0200);
		mask >>=1;
	}
}
//----------------------------------------------------------
void MAX7219::ctrl_flash(uint8_t mask)//прямое управление миганием
{
	uint8_t z;
	for(z=0;z<8;z++)
	{
		if(mask & 0x01)led_buf[z] |= 0x200; else led_buf[z] &= ~(0x0200);
		mask >>=1;
	}
}

//---------------------------------------------
void MAX7219::print ( uint8_t val, uint8_t base)
{
	print ( (uint32_t) val,  base);
}
//---------------------------------------------
void MAX7219::print (int8_t val, uint8_t base)
{
	print ((int32_t) val,  base);
}
//---------------------------------------------
void MAX7219::print ( uint16_t val, uint8_t base)
{
	print ( (uint32_t) val,  base);
}
//---------------------------------------------
void MAX7219::print ( int16_t val, uint8_t base)
{
	print ((int32_t) val,  base);
}
//-------------------------
void MAX7219::print ( int32_t val, uint8_t base)
{
	  if (base == 0) {
		  return ;
		  } else if (base == 10) {
		  if (val < 0) {
			 led_buf[poz++] = G;
			 val = -val;
			// printNumber(val, 10);
		  }
		   printNumber(val, 10);
		  } else {
		   printNumber(val, base);
	  }
}
//-------------------------
void MAX7219::print ( uint32_t val, uint8_t base)
{
	printNumber(val,base);
}
//-------------------------
void MAX7219::printNumber( uint32_t n, uint8_t base) 
{
//	uint8_t buf[8 * sizeof(long) + 1]; // Assumes 8-bit chars plus zero byte.
//	uint8_t *str = &buf[sizeof(buf) - 1];
	uint8_t buf[9]; // Assumes 8-bit chars plus zero byte.
	uint8_t *str = &buf[8];

	*str = 0xff;
	if (base < 2) base = 10;

	do {
		uint8_t c = n % base;
		n /= base;
		*--str = c;
		if(str < &buf[0])break;
	} while(n);
	
	for (;poz<8;poz++)
	{
		if(*str == 0xff) break;
		led_buf[poz] = (led_buf[poz] & 0x0300) | pgm_read_byte(&ZnGen[*str]);
		str++;
	}	
}
//------------------------------------------------------
void MAX7219::print(double number, uint8_t digits)
{
	printFloat(number, digits);
}
//-------------------------------
void MAX7219::printFloat(double number, uint8_t digits)
{
	
	//if (isnan(number)) return print("nan");
	//if (isinf(number)) return print("inf");
//	if (number > 4294967040.0) return print ("ovf");  // constant determined empirically
//	if (number <-4294967040.0) return print ("ovf");  // constant determined empirically
	
	// Handle negative numbers
	if (number < 0.0)
	{
		led_buf[poz++] = G;
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i=0; i<digits; ++i)
	rounding /= 10.0;
	
	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	print(int_part,10);

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0) {
	//	n += print(".");
	led_buf[poz-1] |= H;
	}

	// Extract digits from the remainder one at a time
	while (digits-- > 0)
	{
		remainder *= 10.0;
		int toPrint = int(remainder);
		print(toPrint,10);
		remainder -= toPrint;
	}
	
}

//---------------------------
void MAX7219::print (char val)
{
	led_buf[poz] = (led_buf[poz]& 0x0300)| pgm_read_byte(&ZnGen[(uint8_t)val]);
	poz++;
}
//-----------------------------
void MAX7219::print (char *txt)
{
	while (*txt != 0xff)
	{
		led_buf[poz] = (led_buf[poz]& 0x0300)| pgm_read_byte(&ZnGen[(uint8_t)*txt]);
		poz++;
		txt++;
		if(poz > 7)break;
	}
}
//-----------------------------
void MAX7219::print_F (const char *txt)
{
	uint8_t kd;
	while (1)
	{
		kd= pgm_read_byte(txt);
		if(kd == 0xff || poz > 7)break;
		led_buf[poz] =  (led_buf[poz]& 0xff00)| pgm_read_byte(&ZnGen[kd]);
		poz++;
		txt++;
//		if(poz > 7)break;
	}
}
//--------------------------
void  MAX7219::flash_inv(void)
{
	if(flash_mark == false)flash_mark = true; else flash_mark = false;
}
//--------------------------
