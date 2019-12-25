/*
 * gnc.h
 *
 *  Created on: Dec 19, 2019
 *      Author: yosihisa
 */

#ifndef GNC_H_
#define GNC_H_

#include "cansat.h"


void mode1_Standby(cansat_t* data);		//待機
void mode2_Descent(cansat_t* data);		//降下
void mode3_Separation(cansat_t* data);	//分離
void mode4_Avoidance(cansat_t* data);	//回避
void mode5_Calibration(cansat_t* data); //キャリブレーション
void mode6_GNSS(cansat_t* data);		//GNSS誘導
void mode7_Optical(cansat_t* data);		//光学誘導
void mode8_Goal(cansat_t* data);		//ゴール

#endif /* GNC_H_ */
