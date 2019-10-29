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

#define LPS25H_WRITE 0xBA
#define LPS25H_READ  0xBB

 //----------------------------------------INA226----------------------------------------
unsigned short ina226_who_am_i(); //2260
void set_UVP(const short voltage); //警告電圧設定[mV]
short get_current_raw(); //電流取得
short get_voltage_raw(); //電圧取得
short get_current(); //電流取得[mA]
short get_voltage(); //電圧取得[mV]
unsigned short reset_UVP(); //UVP復帰

 //----------------------------------------LPS25H----------------------------------------
unsigned char lps25h_who_am_i(); //BD
void init_lps25h();
unsigned long get_press();
short get_temp();


 //----------------------------------------ADXL375----------------------------------------


#endif /* I2C_SENSOR_H_ */
