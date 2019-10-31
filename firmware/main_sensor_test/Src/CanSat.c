#include "CanSat.h"
/*
 * CanSat.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */


void init_I2C(){
	set_UVP(6400);
	init_lps25h();
	init_adxl375();
	init_lsm303();
}

void update_sensor(cansat_t* data){
	data->voltage = get_voltage();
	data->current = get_current();

	data->press = get_press();
	data->accel = get_acceleration();
	data->compass = get_compass();

	data->dist_ToF = -1;
}
