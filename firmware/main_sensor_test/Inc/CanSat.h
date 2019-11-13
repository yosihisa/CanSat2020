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
	unsigned long log_num;

	short mode;
	char flightPin;

	short voltage;
	short current;

	struct gnss gnss;

	struct xyza compass;
	struct xyz accel;
	unsigned long press;
	short press_d;

	unsigned short dist_ToF;

	float arg;

	unsigned char nichrome;

	img_t img;
	motor_t motor;

}cansat_t;

void init_I2C();
void update_sensor(cansat_t *data);
void motor(motor_t* motor, short voltage);
void init_pwm(motor_t* motor);
void set_motorSpeed(motor_t* motor,const int8_t L,const int8_t R);

#endif /* CANSAT_H_ */
