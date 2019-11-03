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
	unsigned short xc, yc, s, name;
}img_t;

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

	short motor_L;
	short motor_R;

	short motor_L_ref;
	short motor_R_ref;

	unsigned char nichrome;

	img_t img;

}cansat_t;

void init_I2C();
void update_sensor(cansat_t *data);



#endif /* CANSAT_H_ */
