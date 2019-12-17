/*
 * CanSat.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#ifndef CANSAT_H_
#define CANSAT_H_

#include "main.h"

#include <math.h>

#include "Flash.h"
#include "GNSS.h"
#include "I2C_Sensor.h"
#include "CUI.h"

/*
int8	char
int16	short
int32	long
int64	long long

float	32
double	64
*/

typedef struct {
	unsigned short xc, yc, name;
	uint32_t s;
}img_t;

//目標値 - 100～+ 100 
//操作量 -1000～+1000 (PWMの範囲)
typedef struct {
	int8_t L_ref;
	int8_t R_ref;
	short L;
	short R;
}motor_t;

typedef struct {
	
	unsigned long	flash_address;

	unsigned long	log_num;
	short			mode;
	char			flightPin;
	unsigned char	nichrome;
	short			arg;
	motor_t motor;
	short			voltage;
	short			current;
	struct			gnss gnss;
	unsigned long	press;
	short			press_d;
	struct			xyza compass;
	struct			xyz accel;
	img_t			img;

}cansat_t;


void _motor(motor_t* motor, short voltage);
void motor_Speed(motor_t* motor, const int8_t L, const int8_t R);
void motorStop();

void init_I2C();
void init_pwm(motor_t* motor);

void update_sensor(cansat_t* data);

void write_log(cansat_t* data);

#endif /* CANSAT_H_ */
