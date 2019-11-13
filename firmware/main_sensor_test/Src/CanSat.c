/*
 * CanSat.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#include "CanSat.h"

#define ACCELERATION  20	//最大加速度
#define VOLTAGE_LIMIT 6000	//最大電圧[m/v]

#define TIM_PWM htim3
extern TIM_HandleTypeDef TIM_PWM;


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

	data->dist_ToF = -1;

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
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_4, motor->R);
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_3, 0);
	}
	else {
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_4, 0);
		__HAL_TIM_SetCompare(&TIM_PWM, TIM_CHANNEL_3, -1*motor->R);
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