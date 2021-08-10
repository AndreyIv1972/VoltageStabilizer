/*
 * thermistor_mf59_3950.h
 *
 * Created: 13.02.2020 8:04:57
 *  Author: Andrey
 */ 


#ifndef THERMISTOR_MF59_3950_H_
#define THERMISTOR_MF59_3950_H_


typedef struct tab_temp_s
{
	uint16_t inp;
	int16_t out;
}tab_temp;


// A=0,000694767057889701
// B=0,000219471762460372
// C=9,449656691256E-8

#define THERMISTOR_TABLE_SCALE 1.0f
#define OVERSAMPLENR 1.0
#define OV(N) int16_t((N) * (OVERSAMPLENR) * (THERMISTOR_TABLE_SCALE))
//MF59 100k 4.7k pull-up
const tab_temp temp_table[] PROGMEM =
{
{ OV( 120.358 ), 200 }, /* R=574,694 Om */
{ OV( 142.942 ), 190 }, /* R=700,048 Om */
{ OV( 170.109 ), 180 }, /* R=859,631 Om */
{ OV( 202.641 ), 170 }, /* R=1064,632 Om */
{ OV( 241.310 ), 160 }, /* R=1330,506 Om */
{ OV( 286.775 ), 150 }, /* R=1678,838 Om */
{ OV( 339.428 ), 140 }, /* R=2140,129 Om */
{ OV( 399.185 ), 130 }, /* R=2758,014 Om */
{ OV( 465.288 ), 120 }, /* R=3595,750 Om */
{ OV( 536.143 ), 110 }, /* R=4746,313 Om */
{ OV( 609.324 ), 100 }, /* R=6348,403 Om */
{ OV( 681.794 ), 90 }, /* R=8612,196 Om */
{ OV( 750.353 ), 80 }, /* R=11861,545 Om */
{ OV( 812.182 ), 70 }, /* R=16604,429 Om */
{ OV( 865.322 ), 60 }, /* R=23652,923 Om */
{ OV( 908.898 ), 50 }, /* R=34331,881 Om */
{ OV( 943.067 ), 40 }, /* R=50850,265 Om */
{ OV( 968.759 ), 30 }, /* R=76978,059 Om */
{ OV( 987.334 ), 20 }, /* R=119312,740 Om */
{ OV( 1000.276 ), 10 }, /* R=189715,723 Om */
{ OV( 1008.978 ), 0 }, /* R=310142,247 Om */
{ OV( 1014.631 ), -10 }, /* R=522530,371 Om */
{ OV( 1018.176 ), -20 }, /* R=909767,815 Om */
{ OV( 1020.322 ), -30 }, /* R=1641860,316 Om */
};


#define T_SIZE (sizeof(temp_table)/sizeof(temp_table[0]))

#endif /* THERMISTOR_MF59_3950_H_ */