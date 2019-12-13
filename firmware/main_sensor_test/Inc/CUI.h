/*
 * CUI.h
 *
 *  Created on: 2019/12/12
 *      Author: yosihisa
 */

#ifndef CUI_H_
#define CUI_H_

#include "main.h"
#include "Flash.h"

void CUI_main();

void set_goalFromEEPROM(int num);

void print_goalList();
void print_logList();
#endif /* CUI_H_ */
