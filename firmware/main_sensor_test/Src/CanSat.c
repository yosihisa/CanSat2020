#include "CanSat.h"
/*
 * CanSat.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

void init_I2C(){
	set_UVP(6400);
}

void update_sensor(cansat_t* data){
	data->voltage = get_voltage();
	data->current = get_current();

}
