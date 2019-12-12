/*
 * CanSat.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#include "CanSat.h"
#include <stdio.h>
#include <string.h>


#define ACCELERATION  20	//最大加速度
#define VOLTAGE_LIMIT 7400	//最大電圧[m/v]

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

void motor(motor_t* motor, short voltage) {
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

void set_motorSpeed(motor_t* motor, const int8_t L, const int8_t R) {
	motor->L_ref = L;
	motor->R_ref = R;
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
	sprintf(str, "%5ld,%d,%d,%d,%d, %d,%d,%d,%d, %4d,%3d, %02d,%02d,%02d.%03d,\t",
		data->log_num,		data->mode,			data->flightPin,		data->nichrome,		data->arg,
		data->motor.L_ref,	data->motor.R_ref,	data->motor.L,			data->motor.R,
		data->voltage,		data->current,
		data->gnss.hh,		data->gnss.mm,		data->gnss.ss,			data->gnss.ms
	);
	HAL_UART_Transmit(&COM_PORT, (uint8_t*)str, strlen(str), 10); //COM

	//SUB
	sprintf(str, "%5ld,%d,%d,%d,%d, %d,%d,%d,%d, %4d,%3d, %02d,%02d,%02d.%03d, %d,%8ld,%8ld, %d,%ld,%d, %d,%ld,%d, %d,%d,%d,%ld, %+d,%+d,%+d\n",
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
	HAL_UART_Transmit(&SUB_PORT, (uint8_t*)str, strlen(str), 10); //SUB

	//PC
	sprintf(str, "%5ld M%d F%d N%d A%+4d r(%+4d,%+4d) m(%+4d,%+4d) %4dmV %3dmA %02d:%02d:%02d.%03d,\t",
		data->log_num,		data->mode,			data->flightPin,		data->nichrome,		data->arg,
		data->motor.L_ref,	data->motor.R_ref,	data->motor.L,			data->motor.R,
		data->voltage,		data->current,
		data->gnss.hh,		data->gnss.mm,		data->gnss.ss,			data->gnss.ms
	);
	HAL_UART_Transmit(&PC_PORT, (uint8_t*)str, strlen(str), 10); //PC
	sprintf(str, "G[%c(%8ld,%8ld) S%3d D%8ld(%+4d)] Ca%+4d P[%ld,%d] I[%d.jpg,(%3d,%3d)s%ld] A[%+3d,%+3d,%+3d]\n",
		(data->gnss.state == 1 ? 'A' : 'V'),	data->gnss.latitude,	data->gnss.longitude,
		data->gnss.speed,	data->gnss.dist,	data->gnss.arg,
		data->compass.arg,	data->press,		data->press_d,
		data->img.name,		data->img.xc,		data->img.yc,			data->img.s,
		data->accel.x, data->accel.y, data->accel.z
	);
	HAL_UART_Transmit(&PC_PORT, (uint8_t*)str, strlen(str), 10); //PC


	//COM 後半
	sprintf(str, "%d,%8ld,%8ld, %d,%ld,%d, %d,%ld,%d, %d,%d,%d,%ld, %+d,%+d,%+d\t\n",
		data->gnss.state,	data->gnss.latitude,data->gnss.longitude,
		data->gnss.speed,	data->gnss.dist,	data->gnss.arg,
		data->compass.arg,	data->press,		data->press_d,
		data->img.name,		data->img.xc,		data->img.yc,			data->img.s,
		data->accel.x, data->accel.y, data->accel.z
	);
	HAL_UART_Transmit(&COM_PORT, (uint8_t*)str, strlen(str), 10); //COM

}

void write_log(cansat_t* data) {
	uint8_t buff[256];
	memset(buff, 0xFF, 256);

	print_log(data);

	data->log_num++;

	//data[];
}
