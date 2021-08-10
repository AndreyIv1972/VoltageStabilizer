/*
 * rele_ctrl.h
 *
 * Created: 23.07.2021 14:16:44
 *  Author: Andrey
 */ 


#ifndef RELE_CTRL_H_
#define RELE_CTRL_H_

#define PORT_R PORTC
#define DDR_R DDRC
#define RL1 (1<<PC0)
#define RL2 (1<<PC1)
#define RL3 (1<<PC2)
#define RL4 (1<<PC3)

namespace RELE
{
	enum rmode
	{
		step_0,
		step_1,
		step_2,
		step_3,
		step_4,
		step_5
	};
	void init(void);
	void setmode(float ui);
	void on(void);
	void off(void);
};




#endif /* RELE_CTRL_H_ */