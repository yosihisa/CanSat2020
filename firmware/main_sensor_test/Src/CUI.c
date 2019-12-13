/*
 * CUI.c
 *
 *  Created on: 2019/12/12
 *      Author: yosihisa
 */
#include "CUI.h"
#include <stdio.h>
#include <string.h>

void CUI_main() {
}

void set_goalFromEEPROM(int num) {
	struct gnss_goal goal = read_goal(num);
	set_gnssGoal(&goal);
	printf("Set Goal N=%d [%ld,%ld] %ld[dm]\n", num, goal.latitude, goal.longitude, goal.dist);
	return;
}

void print_goalList() {
	struct gnss_goal goal;
	for (int i = 0; i < 16; i++) {
		goal = read_goal(i);
		printf(" No.%02d [%+10ld,%+10ld] %4ld[dm]\t\n", i, goal.latitude, goal.longitude, goal.dist);
	}
}

void print_logList() {
	unsigned long time;
	unsigned long flash_address;
	unsigned long eeprom_address = DATA_EEPROM_BASE + 0xC0;

	for (int i = 0; eeprom_address < DATA_EEPROM_BASE + 0x17FF; i++, eeprom_address += 8) {
		eeprom_readWord(eeprom_address + 0, &time);
		eeprom_readWord(eeprom_address + 4, &flash_address);
		if (time == 0xFFFFFFFF) {
			break;
		}
		printf(" No.%3d   %02d:%02d:%02d   %04lX \t\n",
			i,
			(uint8_t)(time >> 16),
			(uint8_t)(time >> 8),
			(uint8_t)(time >> 0),
			flash_address
		);
	}
}
