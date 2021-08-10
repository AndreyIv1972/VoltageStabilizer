/*
 * rele_ctrl.cpp
 *
 * Created: 23.07.2021 14:16:24
 *  Author: Andrey
 */ 
#include <avr/io.h>
#include "rele_ctrl.h"



void RELE::on(void){PORT_R |= RL4;}
void RELE::off(void){PORT_R &= ~RL4;}
	
static 	RELE::rmode step;


void RELE::init(void)
{
	PORT_R = (PORT_R & ~(RL1 | RL2 | RL3 | RL4));
	DDR_R |= RL1 | RL2 | RL3 | RL4;
	PORT_R = (PORT_R & ~(RL1 | RL2 | RL3 )) | (RL1 | RL2);//¬ыбор реле на номинальное напр€жение
	step = step_4;
}	
	
void RELE::setmode(float ui)
{
	uint8_t mask;
	switch(step)
	{

		case step_0:
		{
			mask = 0;
			if(ui > 246.1) {step = step_5;break;}
			if(ui > 222.5) {step = step_4;break;}
			if(ui > 202.2) {step = step_3;break;}
			if(ui > 190.1) {step = step_2;break;}
			if(ui > 172.8) {step = step_1;break;}
			break;
		}
		case step_1:
		{
			mask = RL3;
			if(ui > 246.1) {step = step_5;break;}
			if(ui > 222.5) {step = step_4;break;}
			if(ui > 202.2) {step = step_3;break;}
			if(ui > 190.1) {step = step_2;break;}
				
			if(ui < 165.2) {step = step_0;break;}
			break;
		}		
		case step_2:
		{
			mask = RL1;
			if(ui > 246.1) {step = step_5;break;}
			if(ui > 222.5) {step = step_4;break;}
			if(ui > 202.2) {step = step_3;break;}
				
			if(ui < 165.2) {step = step_0;break;}
			if(ui < 175.7) {step = step_1;break;}
			break;
		}		
		case step_3:
		{
			mask = RL1 | RL3;
			if(ui > 246.1) {step = step_5;break;}
			if(ui > 222.5) {step = step_4;break;}
				
			if(ui < 165.2) {step = step_0;break;}
			if(ui < 175.7) {step = step_1;break;}
			if(ui < 193.4) {step = step_2;break;}
			break;
		}
		case step_4:
		{
			mask = RL1 | RL2;
			if(ui > 246.1) {step = step_5;break;}
				
			if(ui < 165.2) {step = step_0;break;}
			if(ui < 175.7) {step = step_1;break;}
			if(ui < 193.4) {step = step_2;break;}
			if(ui < 213.9) {step = step_3;break;}							
			break;
		}		
		case step_5:
		{
			mask = RL1 | RL2| RL3;
			if(ui < 165.2) {step = step_0;break;}
			if(ui < 175.7) {step = step_1;break;}
			if(ui < 193.4) {step = step_2;break;}
			if(ui < 213.9) {step = step_3;break;}
			if(ui < 235.3) {step = step_4;break;}				
			break;
		}
		default:
		{
			mask = 0;
			step = step_0;
			break;
		}
				
	}
	PORT_R = (PORT_R & ~(RL1 | RL2 | RL3 )) | mask;
	

			
}
