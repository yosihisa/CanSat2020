/*
 * CanSat.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#include "CanSat.h"
#include <stdio.h>
#include <string.h>


#define ACCELERATION  10	//最大加速度
#define VOLTAGE_LIMIT 7400	//最大電圧[m/v]

#define PRESS_K1 0.050F		//落下速度計算用係数1
#define PRESS_K2 0.005F		//落下速度計算用係数2

#define TIM_PWM htim3
extern TIM_HandleTypeDef TIM_PWM;


#define PC_PORT huart5
#define COM_PORT huart1
#define SUB_PORT huart4

extern UART_HandleTypeDef PC_PORT;
extern UART_HandleTypeDef COM_PORT;
extern UART_HandleTypeDef SUB_PORT;
void init_I2C() {
	set_UVP(6400);
	init_lps25h();
	init_adxl375();
	init_lsm303();
}

void update_sensor(cansat_t* data) {

	data->voltage = get_voltage();
	data->current = get_current();

	data->press = get_press();
	data->accel = get_acceleration();
	data->compass = get_compass();


	data->flightPin = HAL_GPIO_ReadPin(FLIGHT_PIN_GPIO_Port, FLIGHT_PIN_Pin);

	get_gnss(&data->gnss, 200);

	//方位偏差計算
	data->arg = data->gnss.arg - data->compass.arg;
	if (data->arg > +180)data->arg -= 360;
	if (data->arg < -180)data->arg += 360;

	//降下速度計算
	unsigned long press	= data->calc_press;
	data->calc_press	= (float)((PRESS_K1 * data->press)   + ((1.0 - PRESS_K1) * data->calc_press));
	data->calc_press_d	= data->calc_press - press;
	data->press_d		= (float)((PRESS_K2 * data->press_d) + ((1.0 - PRESS_K2) * data->calc_press_d));

}

short calc_pwmPower(short cv, int8_t reference, short voltage) {
	long pwm;
	short ref = reference * 10;
	if (VOLTAGE_LIMIT < voltage) {
		ref = ref * VOLTAGE_LIMIT / voltage;
	}
	if (ref > cv + ACCELERATION) {
		pwm = cv + ACCELERATION;
	}
	else if (ref < cv - ACCELERATION) {
		pwm = cv - ACCELERATION;
	}
	else {
		pwm = ref;
	}
	return (short)pwm;
}

void _motor(motor_t* motor, short voltage) {
	motor->L = calc_pwmPower(motor->L, motor->L_ref, voltage);
	motor->R = calc_pwmPower(motor->R, motor->R_ref, voltage);

	if (motor->L > 0) {
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_1, motor->L);
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_2, 0);
	}
	else {
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_1, 0);
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_2, -1*motor->L);
	}

	if (motor->R > 0) {
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_3, motor->R);
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_4, 0);
	}
	else {
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_3, 0);
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_4, -1*motor->R);
	}
}

void motor_Speed(motor_t* motor, const int8_t L, const int8_t R) {
	motor->L_ref = L;
	motor->R_ref = R;
}

void motorStop() {
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_1, 0);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_2, 0);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_3, 0);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_4, 0);
}

void init_pwm(motor_t* motor) {
	HAL_TIM_PWM_Start(&TIM_PWM, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&TIM_PWM, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&TIM_PWM, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&TIM_PWM, TIM_CHANNEL_4);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_1, 0);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_2, 0);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_3, 0);
	__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_4, 0);
	motor->L = 0;
	motor->R = 0;
	motor->L_ref = 0;
	motor->R_ref = 0;
}

void print_log(cansat_t* data) {
	char str[512];

	//COM 前半
	HAL_GPIO_WritePin(LED_TX_GPIO_Port, LED_TX_Pin, GPIO_PIN_SET);
	sprintf(str, "%5ld,%d,%d,%d,%d, %d,%d,%d,%d, %4d,%3d, %02d,%02d,%02d.%03d,\t",
		data->log_num,		data->mode,			data->flightPin,		data->nichrome,		data->arg,
		data->motor.L_ref,	data->motor.R_ref,	data->motor.L,			data->motor.R,
		data->voltage,		data->current,
		data->gnss.hh,		data->gnss.mm,		data->gnss.ss,			data->gnss.ms
	);
	HAL_UART_Transmit(&COM_PORT, (uint8_t*)str, strlen(str), 20); //COM
	HAL_GPIO_WritePin(LED_TX_GPIO_Port, LED_TX_Pin, GPIO_PIN_RESET);

	//SUB
	sprintf(str, "%5ld,%d,%d,%d,%d, %d,%d,%d,%d, %4d,%3d, %02d,%02d,%02d.%03d,\t%d,%10ld,%10ld, %d,%ld,%d, %d,%ld,%d, %d,%d,%d,%ld, %+d,%+d,%+d,\r\n",
		data->log_num,		data->mode,			data->flightPin,		data->nichrome,		data->arg,
		data->motor.L_ref,	data->motor.R_ref,	data->motor.L,			data->motor.R,
		data->voltage,		data->current,
		data->gnss.hh,		data->gnss.mm,		data->gnss.ss,			data->gnss.ms,
		data->gnss.state,	data->gnss.latitude,data->gnss.longitude,
		data->gnss.speed,	data->gnss.dist,	data->gnss.arg,
		data->compass.arg,	data->press,		data->press_d,
		data->img.name,		data->img.xc,		data->img.yc,			data->img.s,
		data->accel.x, data->accel.y, data->accel.z
	);
	HAL_UART_Transmit(&SUB_PORT, (uint8_t*)str, strlen(str), 50); //SUB

	//PC
	sprintf(str, "%5ld(0x%04lX) M%d F%d N%d A%+4d r(%+4d,%+4d) m(%+4d,%+4d) %4dmV %3dmA %02d:%02d:%02d.%03d ",
		data->log_num,		data->flash_address,data->mode,				data->flightPin,	data->nichrome,		data->arg,
		data->motor.L_ref,	data->motor.R_ref,	data->motor.L,			data->motor.R,
		data->voltage,		data->current,
		data->gnss.hh,		data->gnss.mm,		data->gnss.ss,			data->gnss.ms
	);
	HAL_UART_Transmit(&PC_PORT, (uint8_t*)str, strlen(str), 20); //PC
	sprintf(str, "G[%c(%10ld,%10ld) S%3d D%8ld(%+4d)] Ca%+4d P[%ld,%d] I[%d.jpg,(%3d,%3d)s%ld] A[%+3d,%+3d,%+3d]\n",
		(data->gnss.state == 1 ? 'A' : 'V'),	data->gnss.latitude,	data->gnss.longitude,
		data->gnss.speed,	data->gnss.dist,	data->gnss.arg,
		data->compass.arg,	data->press,		data->press_d,
		data->img.name,		data->img.xc,		data->img.yc,			data->img.s,
		data->accel.x, data->accel.y, data->accel.z
	);
	HAL_UART_Transmit(&PC_PORT, (uint8_t*)str, strlen(str), 20); //PC


	//COM 後半
	HAL_GPIO_WritePin(LED_TX_GPIO_Port, LED_TX_Pin, GPIO_PIN_SET);
	sprintf(str, "%d,%10ld,%10ld, %d,%ld,%d, %d,%ld,%d, %d,%d,%d,%ld, %+d,%+d,%+d,\t\n",
		data->gnss.state,	data->gnss.latitude,data->gnss.longitude,
		data->gnss.speed,	data->gnss.dist,	data->gnss.arg,
		data->compass.arg,	data->press,		data->press_d,
		data->img.name,		data->img.xc,		data->img.yc,			data->img.s,
		data->accel.x, data->accel.y, data->accel.z
	);
	HAL_UART_Transmit(&COM_PORT, (uint8_t*)str, strlen(str), 20); //COM
	HAL_GPIO_WritePin(LED_TX_GPIO_Port, LED_TX_Pin, GPIO_PIN_RESET);
}

void write_log(cansat_t* data) {
	uint32_t buff[64];
	memset(buff, 0xFF, sizeof(buff));
	uint8_t unit = (data->log_num % 3) * 20;
	print_log(data);
	buff[unit +  0] = (data->log_num);
	buff[unit +  1] = (data->mode		<< 16) + (data->flightPin	<< 8) + (data->nichrome);
	buff[unit +  2] = (data->arg		<< 16) + (data->motor.L_ref << 8) + (data->motor.R_ref);
	buff[unit +  3] = (data->motor.L	<< 16) + (data->motor.R);
	buff[unit +  4] = (data->voltage	<< 16) + (data->current);
	buff[unit +  5] = (data->gnss.latitude);
	buff[unit +  6] = (data->gnss.longitude);
	buff[unit +  7] = (data->gnss.hh	<< 24) + (data->gnss.mm		<< 16) + (data->gnss.ss << 8) + (data->gnss.state);
	buff[unit +  8] = (data->gnss.ms	<< 16) + (data->gnss.speed);
	buff[unit +  9] = (data->gnss.dist);
	buff[unit + 10] = (data->gnss.arg	<< 16) + (data->press_d);
	buff[unit + 11] = (data->press);
	buff[unit + 12] = (data->compass.x	<< 16) + (data->compass.y);
	buff[unit + 13] = (data->compass.z	<< 16) + (data->compass.arg);
	buff[unit + 14] = (data->accel.x	<< 16) + (data->accel.y);
	buff[unit + 15] = (data->accel.z	<< 16) + (data->img.name);
	buff[unit + 16] = (data->img.xc		<< 16) + (data->img.yc);
	buff[unit + 17] = (data->img.s);
	buff[unit + 18] = 0;
	buff[unit + 19] = 0;

	buff[63] = 0;

	flash_write_page(data->flash_address, (uint8_t*)&buff[0]);
	
	if (data->log_num % 3 == 2) {
		data->flash_address++;
	}

	data->log_num++;
}
