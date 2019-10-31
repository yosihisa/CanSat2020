/*
 * I2C_sensor.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#ifndef I2C_SENSOR_H_
#define I2C_SENSOR_H_

#include "CanSat.h"

#define INA226_WRITE  0x80
#define INA226_READ   0x81
#define LPS25H_WRITE  0xBA
#define LPS25H_READ   0xBB
#define ADXL375_WRITE 0xA6
#define ADXL375_READ  0xA7
#define LSM303_WRITE  0x3C
#define LSM303_READ   0x3D

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
unsigned char adxl375_who_am_i(); //BD
void init_adxl375();
struct xyz get_acceleration();

 //----------------------------------------LSM303AGR-C------------------------------------
unsigned char lsm303_who_am_i(); //BD
void init_lsm303();
void set_lsm303_offset(struct xyz offset);
struct xyza get_compass();

#endif /* I2C_SENSOR_H_ */
