/*
 * GNSS.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#ifndef GNSS_H_
#define GNSS_H_

#include "stm32l0xx_hal.h"

#define GNSS_PORT huart2
extern UART_HandleTypeDef GNSS_PORT;

struct gnss {
	int32_t latitude;
	int32_t longitude;
	uint8_t hh, mm, ss;
	uint16_t ms;
	uint16_t speed;

	uint8_t state; //0:無効 1:有効

	int16_t arg;
	uint32_t dist;
};

//目的座標と半径
struct gnss_goal {
	int32_t latitude;
	int32_t longitude;
	uint32_t dist;
};


void init_gnss();
void set_gnssGoal(const int32_t latitude, const int32_t longitude, const uint64_t dist);
void get_gnssGoal(int32_t *latitude, int32_t *longitude, uint64_t *dist);
int get_gnss(struct gnss* data, const uint32_t TIMEOUT);

#endif /* GNSS_H_ */
