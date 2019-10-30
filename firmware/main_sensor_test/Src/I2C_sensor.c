/*
 * I2C_sensor.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#include "I2C_sensor.h"

#define ABS(x) ( x>=0 ? x : -1*x )

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



//----------------------------------------LPS25H----------------------------------------
#define LPS25H_REF_P_XL			0x08
#define LPS25H_REF_P_L			0x09
#define LPS25H_REF_P_H			0x0A
#define LPS25H_WHO_AM_I			0x0F
#define LPS25H_RES_CONF			0x10
#define LPS25H_CTRL_REG1		0x20
#define LPS25H_CTRL_REG2		0x21
#define LPS25H_CTRL_REG3		0x22
#define LPS25H_CTRL_REG4		0x23
#define LPS25H_INT_CFG			0x24
#define LPS25H_INT_SOURCE		0x25
#define LPS25H_STATUS_REG		0x27
#define LPS25H_PRESS_OUT_XL		0x28
#define LPS25H_PRESS_OUT_L		0x29
#define LPS25H_PRESS_OUT_H		0x2A
#define LPS25H_TEMP_OUT_L		0x2B
#define LPS25H_TEMP_OUT_H		0x2C
#define LPS25H_FIFO_CTRL		0x2E
#define LPS25H_FIFO_STATUS		0x2F
#define LPS25H_THS_P_L			0x30
#define LPS25H_THS_P_H			0x31
#define LPS25H_RPDS_L			0x39
#define LPS25H_RPDS_H			0x3A

unsigned char lps25h_who_am_i() {
	uint8_t rx_buff;
	HAL_I2C_Mem_Read(&I2C_PORT, LPS25H_READ, LPS25H_WHO_AM_I, 1, &rx_buff, 1, 10);
	return rx_buff;
}

void init_lps25h() {
	uint8_t tx_buff;
	tx_buff = 0x0F;
	HAL_I2C_Mem_Write(&I2C_PORT, LPS25H_WRITE, LPS25H_RES_CONF, 1, &tx_buff, 1, 10);

	tx_buff = 0b11000000;
	HAL_I2C_Mem_Write(&I2C_PORT, LPS25H_WRITE, LPS25H_CTRL_REG1, 1, &tx_buff, 1, 10);
}

unsigned long get_press() {
	unsigned long press = 0;
	uint8_t rx_buff;

	HAL_I2C_Mem_Read(&I2C_PORT, LPS25H_READ, LPS25H_PRESS_OUT_H, 1, &rx_buff, 1, 10);
	press = rx_buff << 16;
	HAL_I2C_Mem_Read(&I2C_PORT, LPS25H_READ, LPS25H_PRESS_OUT_L, 1, &rx_buff, 1, 10);
	press |= rx_buff << 8;
	HAL_I2C_Mem_Read(&I2C_PORT, LPS25H_READ, LPS25H_PRESS_OUT_XL, 1, &rx_buff, 1, 10);
	press |= rx_buff;

	return press;
}

short get_temp() {
	short temp = 0;
	uint8_t rx_buff;

	HAL_I2C_Mem_Read(&I2C_PORT, LPS25H_READ, LPS25H_TEMP_OUT_H, 1, &rx_buff, 1, 10);
	temp |= rx_buff << 8;
	HAL_I2C_Mem_Read(&I2C_PORT, LPS25H_READ, LPS25H_TEMP_OUT_L, 1, &rx_buff, 1, 10);
	temp |= rx_buff;

	return temp;
}


//----------------------------------------ADXL375----------------------------------------
#define ADXL375_DEVID			0x00 //0xE5
#define ADXL375_POWER_CTL		0x2D
#define ADXL375_DATA_FORMAT		0x31
#define ADXL375_BW_RATE			0x2C
#define ADXL375_DATAX0			0x32
#define ADXL375_DATAX1			0x33
#define ADXL375_DATAY0			0x34
#define ADXL375_DATAY1			0x35
#define ADXL375_DATAZ0			0x36
#define ADXL375_DATAZ1			0x37
#define ADXL375_FIFO_CTL		0x38


unsigned char adxl375_who_am_i() {
	uint8_t rx_buff;
	HAL_I2C_Mem_Read(&I2C_PORT, ADXL375_READ, ADXL375_DEVID, 1, &rx_buff, 1, 10);
	return rx_buff;
}

void init_adxl375() {
	uint8_t tx_buff;

	tx_buff = 0x08;
	HAL_I2C_Mem_Write(&I2C_PORT, ADXL375_WRITE, ADXL375_POWER_CTL, 1, &tx_buff, 1, 10);

	tx_buff = 0x0B;
	HAL_I2C_Mem_Write(&I2C_PORT, ADXL375_WRITE, ADXL375_DATA_FORMAT, 1, &tx_buff, 1, 10);

	tx_buff = 0b10000000; //FIFOをstreamモードで使用
	HAL_I2C_Mem_Write(&I2C_PORT, ADXL375_WRITE, ADXL375_FIFO_CTL, 1, &tx_buff, 1, 10);

	tx_buff = 0b00001011; //200G
	HAL_I2C_Mem_Write(&I2C_PORT, ADXL375_WRITE, ADXL375_BW_RATE, 1, &tx_buff, 1, 10);
}

struct xyz get_acceleration() {
	uint8_t rx_buff[6];
	struct xyz res = { 0 };

	//FIFOからデータを読み出し 最大値のみを記録
	for (int i = 0; i < 32; i++) {
		struct xyz data = { 0 };

		HAL_I2C_Mem_Read(&I2C_PORT, ADXL375_READ, ADXL375_DATAX0, 1, rx_buff, 6, 100);
		data.x = rx_buff[0] | (rx_buff[1] << 8);
		data.y = rx_buff[2] | (rx_buff[3] << 8);
		data.z = rx_buff[4] | (rx_buff[5] << 8);

		if (ABS(data.x) > res.x)res.x = data.x;
		if (ABS(data.y) > res.y)res.y = data.y;
		if (ABS(data.z) > res.z)res.z = data.z;

	}
	return res;
}

//----------------------------------------LSM303AGR-C------------------------------------

//----------------------------------------VL53L0X----------------------------------------