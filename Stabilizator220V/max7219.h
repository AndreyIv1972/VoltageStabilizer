/* 
* MAX7219.h
*
* Created: 18.04.2018 11:23:22
* Author: Andrey
*/


#ifndef __MAX7219_H__
#define __MAX7219_H__
#include <avr/io.h>

#define CLK_PORT PORTD
#define STB_PORT PORTD
#define DIO_PORT PORTD
//#define DIO_PPIN PIND

#define CLK_DDR DDRD
#define STB_DDR DDRD
#define DIO_DDR DDRD


#define CLC_PIN PD5
#define STB_PIN PD4
#define DIO_PIN PD2

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2


class MAX7219
{
//variables
public:
//uint8_t key_cod;
protected:
private:
bool flash_mark;
uint8_t brightness;//яркость
uint8_t poz; //позиция для символа 0...7
uint16_t led_buf[8];


//functions
public:
void init(void);

void clear(uint8_t n = 0, uint8_t z = 8);

//void set_led(uint8_t mask);//зажеч светодиоды по маске
//void clr_led(uint8_t mask);//погасить светодиоды по маске
//void ctrl_led(uint8_t mask);//прямое управление светодиодами

void set_flash(uint8_t mask);//мигать по маске
void clr_flash(uint8_t mask);//выкл. мигание по маске
void ctrl_flash(uint8_t mask);//прямое управление миганием

void print ( uint8_t val, uint8_t base = DEC);
void print (int8_t val, uint8_t base = DEC);

void print (uint16_t val, uint8_t base = DEC);
void print (int16_t val, uint8_t base = DEC);

void print (uint32_t val, uint8_t base = DEC);
void print (int32_t val, uint8_t base = DEC);
void print(double number, uint8_t zn = 2); 
void print (char *txt);
void print (char val);
void print_F (const char *txt);

void display(void);//обновление дисплея
void set_brightness(uint8_t br = 15);//установить яркость 0...15
void set_cur(uint8_t cur);
void flash_inv(void);
//void loop(uint32_t nt);

	MAX7219();
	~MAX7219();
protected:
private:
//uint8_t read(void);
//void send(uint8_t val);
void send(uint16_t val);
void send_cmd(uint8_t reg,uint8_t val);
//void send_cmd(uint8_t val, uint16_t *dat,uint8_t len);
void printFloat(double number, uint8_t digits);
void printNumber(uint32_t n, uint8_t base);
//void deley(void);
}; //MAX7219

#endif //__MAX7219_H__
