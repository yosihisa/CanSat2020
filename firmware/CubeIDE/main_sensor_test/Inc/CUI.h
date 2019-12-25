/*
 * CUI.h
 *
 *  Created on: 2019/12/12
 *      Author: yosihisa
 */

#ifndef CUI_H_
#define CUI_H_

void CUI_main();

void set_goalFromEEPROM(int num);

void print_goalList();
void print_logList();

char get_char(int timeout);
#endif /* CUI_H_ */
