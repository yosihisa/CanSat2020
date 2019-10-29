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
void set_UVP(const short voltage); //�x���d���ݒ�[mV]
short get_current_raw(); //�d���擾
short get_voltage_raw(); //�d���擾
short get_current(); //�d���擾[mA]
short get_voltage(); //�d���擾[mV]

unsigned short reset_UVP(); //UVP���A

#endif /* I2C_SENSOR_H_ */
