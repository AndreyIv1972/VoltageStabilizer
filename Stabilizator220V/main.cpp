/*
 * VoltageRegulator220V.cpp
 *
 * Created: 22.06.2021 18:10:05
 * Author : Andrey
 */ 
#define PLATA 1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <math.h>
#include "rele_ctrl.h"
//#include "sys_timer.h"
#include "max7219.h"
#include "thermistor_mf59_3950.h"
#include "LowPassFilter.h"


#define fan_on() PORTB |= (1<<PB0);
#define fan_off() PORTB &= ~(1<<PB0);

static MAX7219 led;
static LowPassFilterFloat ftin;
static LowPassFilterFloat ftout;
static uint8_t error_flag;
//---------------------------

//---------------------------
struct	ResultDataStructure 
{ 
	float  Ui, Uo, Tp;
} Result;


struct   AccDataStructure    
{
volatile int32_t U1, U2, NTC;
}  Accumulator,Sum;


struct   SampleStructure 
{
volatile 	int16_t Fresh; 
volatile 	int16_t Previous;
volatile 	int32_t Filtered;
volatile 	int32_t PreviousFiltered;
volatile 	int32_t Calibrated;
} Sample[3];



#define ADC_VREF_TYPE (1<<REFS0) /*Опорное напряжение = VCC*/

const unsigned char adch[]PROGMEM ={7,4,6};//список измеряемых ацп каналов

//Это количество выборок, которые необходимо накапливать в течение каждого цикла вычислений.
//Большие значения дают более стабильные результаты, но с более длительными интервалами. 
//Должно быть целым числом, кратным количеству отсчетов на цикл сети.
//#define NMAX	401

//#define NMAX	641 /*цикл измерения 200 мС*/
//#define NMAX	321 /**цикл измерения 100 мС**/
#define NMAX	64 /**цикл измерения 20 мС**/
#define NORM    1.0/NMAX

volatile bool cycle_full;
volatile uint8_t groggy;
static bool operate_en;
static bool operate_en_old;
static uint8_t sh_delay;
//#define GROGGY        0x02
//#define CYCLE_FULL    0x08
//Постоянное смещение, которое добавляется ко всем результатам измерений. Улучшает линейность на низких
//амплитуды, если правильно масштабированы. Должно быть около 1/2 LSB x 255 = 128.
//#define OFFSET        0x80
#define OFFSET        0x20
//------------------
float get_tempf(uint16_t adc)
{
	uint8_t l = 0, r = T_SIZE, m;
	for (;;)
	{
		m = (l + r) >> 1;
		if (!m) return (float)((int16_t)pgm_read_word(&temp_table[0].out));
		if (m == l || m == r) return (float)((int16_t)pgm_read_word(&temp_table[T_SIZE-1].out));
		uint16_t v00 = pgm_read_word(&temp_table[m-1].inp);
		uint16_t v10 = pgm_read_word(&temp_table[m-0].inp);
		if (adc < v00) r = m;
		else if (adc > v10) l = m;
		else {
			const int16_t v01 = int16_t(pgm_read_word(&temp_table[m-1].out));
			const int16_t v11 = int16_t(pgm_read_word(&temp_table[m-0].out));
			return (float)((float)v01 + float(adc - v00) * float(v11 - v01) / float(v10 - v00));
		}
	}
}
//---------------------------

//-------------------------Измерение напряжения
ISR(ADC_vect)
{
	static	uint16_t SampleCounter;
	static	uint8_t Index;
	static	int32_t Temp[3];
	int16_t	TempI;
	int32_t TempL;
  
    Sample[Index].Previous=Sample[Index].Fresh;	// x[n+1] <- x[n]
    Sample[Index].Fresh=ADC;            // save voltage sample as is

// Применяем фильтр для удаления смещения постоянного тока. 
// Для точности необходимо использовать длинные типы данных! 
// Передаточная функция выглядит следующим образом:
// y[n] = 0.996*y[n-1] + 0.996*x[n] - 0.996*x[n-1]

  Sample[Index].PreviousFiltered=Sample[Index].Filtered;  // y[n] <- y[n-1]
  TempL=255*(int32_t)Sample[Index].Filtered;
  TempL=TempL>>8;
  TempI=Sample[Index].Fresh-Sample[Index].Previous;
  TempL=TempL+255*(int32_t)TempI;
  Sample[Index].Filtered=TempL;

//Коррекция сдвига по фазе
  //TempL=Sample[Index].Filtered-Sample[Index].PreviousFiltered;
  //TempL=TempL*CalCoeff.PCC[Index];
  //TempL=TempL>>16;
  //Sample[Index].Calibrated=Sample[Index].Filtered-TempL;
  
  Sample[Index].Calibrated=Sample[Index].Filtered;

  Temp[Index]=Sample[Index].Calibrated>>6;
	
  switch (Index)
  {
	  case 0: // Index=0 => ADC6 (Voltage IN)
	  Accumulator.U1=Accumulator.U1+(Temp[0]*Temp[0]);
	  break;
	  case 1: // Index=1 => ADC7 (Voltage OUT)
	  Accumulator.U2=Accumulator.U2+(Temp[1]*Temp[1]);
	  break;
	  case 2: // Index=2 => ADC4 (NTC)
	  Accumulator.NTC=Accumulator.NTC+Sample[2].Fresh;
	  //Accumulator.IL1=Accumulator.IL1+(Temp[2]*Temp[2]);
	  //Temp[3]=Temp[1]*Temp[2];
	  //Accumulator.PL=Accumulator.PL+Temp[3];
	  SampleCounter++;
	  break;
  }
  
  
    if (SampleCounter==NMAX)
    {
	    SampleCounter=0;
	    if (groggy > 0)
	    {
		    Accumulator.U1=0;
		    Accumulator.U2=0;
		    Accumulator.NTC=0;
		    groggy -- ;          //
	    }
	    else
	    cycle_full = true;
    }
 
    if (++Index > 2)
			Index=0;
 //   ADMUX=ADC_VREF_TYPE+Index;		// Select next ADC input
	ADMUX=ADC_VREF_TYPE + pgm_read_byte(&adch[Index]);
}
//------------------------
void limit_check(void)
{

	//------------------------------------------Проверка датчика температуры
	if(Result.Tp > 195)error_flag |= 0x02; // Замыкание датчика температуры
	if(Result.Tp < 190)error_flag &= ~0x02; 
	if(Result.Tp < -25)error_flag |= 0x04; // Обрыв датчика температуры
	if(Result.Tp > -20)error_flag &= ~0x04;
	//-------------------------------------------
	if((error_flag && 0x06) == 0)	//---------------если датчиктемпературы в норме
	{
		//------------------------------------------Управляем вентилятором
		if(Result.Tp > 50.0) fan_on();
		if(Result.Tp < 40.0) fan_off();
		//------------------------------------------Проверка на перегрев
		if(Result.Tp > 70.0)error_flag |= 0x01;
		if(Result.Tp < 50.0)error_flag &= ~0x01;		
	}
	//-------------------------------------------Проверка пределов сетевого напряжения
	if(Result.Ui < 150.0) error_flag |= 0x08; // Напряжение ниже минимального
	if(Result.Ui > 155.0) error_flag &= ~0x08;
	
	if(Result.Ui > 270.0) error_flag |= 0x10; // Напряжение выше максимального
	if(Result.Ui < 265.0) error_flag &= ~0x10;	
	//------------------------------------------
	//--------------------------------------
	if((error_flag & 0xff)==0)operate_en = true; else operate_en = false;
	//--------------------------------------	
		if(operate_en != operate_en_old)
		{
			operate_en_old = operate_en;
			sh_delay = 0;
		}
		if(sh_delay < 255) sh_delay ++;
		
		if((operate_en == true)&&(sh_delay >150))RELE::on(); // включаем через 3 секунды
		if((operate_en == false)&&(sh_delay >9))RELE::off(); // отключаем через 0.2 сек
	
	//---------------				
					
}
//==========================================================================================
int main(void)
{
uint32_t	TempUL;

#if defined (MCUSR) // Для мега328
uint8_t mcusr = MCUSR & 0x0f;
MCUSR = 0;
#elif defined (MCUCSR) // Для мега8
uint8_t mcusr = MCUCSR;
MCUCSR = 0;
#else
#error Error CPU setting
#endif

wdt_reset(); 
wdt_enable(WDTO_2S);
led.init();
led.set_cur(1);
led.print((char)0x16);
led.print((char)0x0E);
led.print((char)0x1E);
led.print((char)0x1E);
led.print((char)0);
led.display();

ADMUX=ADC_VREF_TYPE + pgm_read_byte(&adch[0]);

#if defined (ADFR)
ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADFR)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
#elif defined (ADATE)
ADCSRA = (1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
#else
#error Error adc setting
#endif

ftin.set_time_constant (0.02, 0.5); //фильтр входного напряжения
ftout.set_time_constant(0.02, 0.5); //фильтр выходного напряжения

groggy = 50; 	//Пропустим первые замеры
RELE::init();



sei();

if(mcusr & (1<<PORF))//Флаг сброса при включении питания
{
	
}
if(mcusr & (1<<EXTRF))//Флаг внешнего сброса
{
	
}
if(mcusr & (1<<BORF))//Флаг сброса при понижении питания
{
	
}
if(mcusr & (1<<WDRF))//Флаг сброса сторожевым таймером
{
	
}

DDRB |= (1<<PB0);//Порт вентилятора

    while (1) 
    {
	
		    if (cycle_full == true)
		    {
			    cycle_full = false;
			    Sum=Accumulator;

			    Accumulator.U1=0;
			    Accumulator.U2=0;
			    Accumulator.NTC=0;
				
				TempUL=(Sum.U1+OFFSET)*NORM;
				Result.Ui=sqrt(TempUL);
				TempUL=(Sum.U2+OFFSET)*NORM;
				Result.Uo=sqrt(TempUL);
				Result.Tp=Sum.NTC*NORM;

//Масштабируем.
#if PLATA == 1
// Коэффициенты для первой платы
				Result.Ui = Result.Ui * 0.27145;
				Result.Uo = Result.Uo * 0.26722;
// Коэффициенты для второй платы
#elif PLATA == 2
				Result.Ui = Result.Ui * 0.27223;
				Result.Uo = Result.Uo * 0.27033;
				
#else
#error error PLATA Selection
#endif				
//--Масштаб температуры по таблице				
				Result.Tp = get_tempf((uint16_t)Result.Tp);

//----------------------------------
				RELE::setmode(Result.Ui);
				//RELE::setmode(200);
				Result.Ui = ftin.apply(Result.Ui);
				Result.Uo = ftout.apply(Result.Uo);
//---------------------------------
				limit_check();

//---------------------------------
				led.clear();
				led.set_cur(0);
				led.print(Result.Ui,0);
				if(error_flag == 0)
				{
				led.set_cur(5);
				led.print(Result.Uo,0);					
				} else
					{
						led.set_cur(4);
						led.print((char)0x0e);
						led.set_cur(6);
						if(error_flag < 0x10)led.print(0);
						led.print(error_flag,HEX);
					}


				led.display();
//---------------------------------	
				wdt_reset();			
			}

    }
}

