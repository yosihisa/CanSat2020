/*
 * I2C_sensor.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#ifndef I2C_SENSOR_H_
#define I2C_SENSOR_H_

#include "CanSat.h"

#define INA226_WRITE 0x80
#define INA226_READ  0x81


unsigned short ina226_who_am_i();
void set_UVP(const short voltage); //警告電圧設定[mV]
short get_current_raw(); //電流取得
short get_voltage_raw(); //電圧取得
short get_current(); //電流取得[mA]
short get_voltage(); //電圧取得[mV]

unsigned short reset_UVP(); //UVP復帰

#endif /* I2C_SENSOR_H_ */
