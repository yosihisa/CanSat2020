/*
 * I2C_sensor.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#include "I2C_sensor.h"

//----------------------------------------INA226----------------------------------------
#define CONFIGURATION	0x00
#define CURRENT			0x01
#define BUS_VOLTAGE		0x02
#define POWER			0x03
#define MASK_ENABLE		0x06
#define ALERT_LIMIT		0x07
#define WHO_AM_I		0xFF //0x2260

unsigned short ina226_who_am_i() {
	uint8_t rx_buff[2];
	HAL_I2C_Mem_Read(&I2C_PORT, INA226_READ, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, rx_buff, 2, 50);
	uint16_t res = (uint16_t)(rx_buff[0] << 8) | rx_buff[1];
	return res;
}

void set_UVP(const  short voltage) {
	uint8_t tx_buff[2];
	tx_buff[0] = 0b01100101;
	tx_buff[1] = 0b10110111;

	HAL_I2C_Mem_Write(&I2C_PORT, INA226_WRITE, CONFIGURATION, I2C_MEMADD_SIZE_8BIT, tx_buff, 2, 50);

	tx_buff[0] = 0b00010000;
	tx_buff[1] = 0b00000000;

	HAL_I2C_Mem_Write(&I2C_PORT, INA226_WRITE, MASK_ENABLE, I2C_MEMADD_SIZE_8BIT, tx_buff, 2, 50);


	int16_t ocp = (int16_t)(voltage / 1.25);
	tx_buff[0] = (uint8_t)(ocp >> 8);
	tx_buff[1] = (uint8_t)ocp;

	HAL_I2C_Mem_Write(&I2C_PORT, INA226_WRITE, ALERT_LIMIT, I2C_MEMADD_SIZE_8BIT, tx_buff, 2, 50);

	return;
}

short get_current_raw() {
	uint8_t rx_buff[2];
	HAL_I2C_Mem_Read(&I2C_PORT, INA226_READ, CURRENT, I2C_MEMADD_SIZE_8BIT, rx_buff, 2, 50);
	int16_t res = (uint16_t)(rx_buff[0] << 8) | rx_buff[1];
	return res;
}

short get_voltage_raw() {
	uint8_t rx_buff[2];
	HAL_I2C_Mem_Read(&I2C_PORT, INA226_READ, BUS_VOLTAGE, I2C_MEMADD_SIZE_8BIT, rx_buff, 2, 50);
	int16_t res = (uint16_t)(rx_buff[0] << 8) | rx_buff[1];
	return res;
}

short get_current() {
	return 0.0025 * 100 * get_current_raw();
}

short get_voltage() {
	return 1.25 * get_voltage_raw();
}

unsigned short reset_UVP() {
	uint8_t rx_buff[2];
	HAL_I2C_Mem_Read(&I2C_PORT, INA226_READ, MASK_ENABLE, I2C_MEMADD_SIZE_8BIT, rx_buff, 2, 50);
	uint16_t res = (rx_buff[0] << 8) + rx_buff[1];
	return res;
}