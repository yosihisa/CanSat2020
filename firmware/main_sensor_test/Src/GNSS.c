/*
 * GNSS.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */


#include "GNSS.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MSG_MAX 100
struct gnss_goal goal = { 0 };
long sr = 0;

int32_t ddmm2ddddd(int32_t ddmm, int32_t mmmm,uint8_t s) {
	int32_t d, m;
	int32_t d_d;
	d = ddmm / 100;
	m = (ddmm % 100) * 10000 + mmmm;
	m *= 100;
	m /= 60;

	d_d = d * 1000000 + m;

	if (s == 'S' || s == 'W') {
		d_d *= -1;
	}
	return d_d;

}

void init_gnss() {
	char str[100];
	HAL_Delay(500);
	sprintf(str, "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n");
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	
	sprintf(str, "$PMTK300,100,0,0,0,0*2C\r\n");
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);

	sprintf(str, "$PMTK220,200*2C\r\n");
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);
	HAL_UART_Transmit(&GNSS_PORT, (uint8_t*)str, strlen(str), 500);
	HAL_Delay(80);

}

void set_gnssGoal(const struct gnss_goal *data) {
	goal.latitude = data->latitude;
	goal.longitude = data->longitude;
	goal.dist = data->dist;
	sr = cosf((float)data->latitude / 1000000)*1000;
	if (sr < 0)sr *= -1;
}

void get_gnssGoal(int32_t* latitude, int32_t* longitude, uint64_t* dist) {
	*latitude = goal.latitude;
	*longitude = goal.longitude;
	*dist = goal.dist;
}


int get_gnss(struct gnss* data, const uint32_t TIMEOUT) {
	data->latitude = 0;
	data->longitude = 0;
	data->hh = 0;
	data->mm = 0;
	data->ss = 0;
	data->ms = 0;
	data->state = -1;
	data->speed = 0;
	data->arg = 0;
	data->dist = -1;

	int timeout = 0;

	uint8_t c;
	char msg[MSG_MAX] = { 0 };
	int time, ms;
	int32_t latH, latL, lonH, lonL;
	uint8_t latS, lonS;
	int knotH, knotL;

	while (1) {

		if (HAL_UART_Receive(&GNSS_PORT, &c, 1, 10) != HAL_OK) {
			timeout++;
			if (TIMEOUT < timeout)return -1;
			continue;
		}
		if (c != '$') {
			continue;
		}

		for (int i = 0; i < MSG_MAX; i++) {
			HAL_UART_Receive(&GNSS_PORT, &c, 1, 20);
			msg[i] = c;
			if (msg[i] == '\n')break;
		}
		if (msg[0] == 'G' && msg[2] == 'R' && msg[3] == 'M' && msg[4] == 'C' && msg[5] == ',') {
			for (int i = 0; i < MSG_MAX - 6; i++) {
				msg[i] = msg[i+6];
			}
			//printf("%s", msg);
			sscanf(msg, "%d.%d,%c,", &time, &ms, &c);
			data->hh = time / 10000;
			data->mm = (time / 100) % 100;
			data->ss = time % 100;
			data->ms = ms;
			data->state = (c == 'A' ? 1 : 0);

			if (c == 'A') {
				sscanf(msg, "%d.%d,%c,%ld.%ld,%c,%ld.%ld,%c,%d.%d,", &time, &ms, &c, &latH, &latL, &latS, &lonH, &lonL, &lonS, &knotH, &knotL);

				data->latitude = ddmm2ddddd(latH, latL, latS);
				data->longitude = ddmm2ddddd(lonH, lonL, lonS);
				data->speed = (knotH * 10 + knotL) * 0.514444; //knot -> dm/s

				int64_t dx = (goal.longitude - data->longitude);
				int64_t dy = (goal.latitude - data->latitude);
				dx *= sr;
				dy *= 1000;

				data->arg = atan2f(dy, dx) * 180.0 / 3.141593;
				data->dist = sqrt(dx * dx + dy * dy) / 900;
			}
			return 0;
		}
	}
}
