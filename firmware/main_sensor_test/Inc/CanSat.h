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

#define I2C_PORT hi2c1

extern I2C_HandleTypeDef I2C_PORT;


/*
int8	char
int16	short
int32	long
int64	long long

float	32
double	64
*/
struct xyza {
	short x, y, z;
	short x_offset;
	short y_offset;
	short z_offset;
	float arg;
};

struct xyz {
	short x, y, z;
};

struct gnss {
	long long latitude;
	long long longitude;
	unsigned char hh, mm, ss;
	unsigned short ms;

	int mode;

	float arg;
	unsigned long long dist;
};

typedef struct {
	unsigned long log_num;

	short mode;
	char flightPin;

	short voltage;
	short current;

	struct gnss gnss_data;
	struct xyza c_data;
	struct xyz a_data;
	unsigned long press;
	short press_d;

	unsigned short dist;

	float arg;

	short motor_L;
	short motor_R;

	short motor_L_ref;
	short motor_R_ref;

	unsigned char nichrome;

}cansat_t;

void init_I2C();
void update_sensor(cansat_t *data);



#endif /* CANSAT_H_ */
