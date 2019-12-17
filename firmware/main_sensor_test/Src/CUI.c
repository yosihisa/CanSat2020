/*
 * CUI.c
 *
 *  Created on: 2019/12/12
 *      Author: yosihisa
 */
#include "main.h"
#include "CUI.h"
#include "Flash.h"
#include "GNSS.h"
#include <stdio.h>
#include <string.h>

#define PC_PORT huart5
#define COM_PORT huart1
#define SUB_PORT huart4

extern UART_HandleTypeDef PC_PORT;
extern UART_HandleTypeDef COM_PORT;
extern UART_HandleTypeDef SUB_PORT;

//1文字取得
char get_char(int timeout) {
	char buff = 0;
	HAL_UART_Receive(&COM_PORT, (uint8_t*)& buff, 1, timeout);
	return buff;
}

//数字を取得 タイムアウトは秒単位で指定
int get_num(int timeout) {
	char c;
	while (timeout > 0) {
		c = get_char(1000);
		if (c >= '0' && c <= '9') {
			HAL_UART_Transmit(&COM_PORT, (uint8_t*)&c, 1, 10);
			HAL_UART_Transmit(&PC_PORT, (uint8_t*)&c, 1, 10);

			return c - '0';
		}
		timeout--;
	}
	printf("input error\n\t");
	return -1;
}

//符号を取得 タイムアウトは秒単位で指定
int get_sign(int timeout) {
	char c;
	while (timeout > 0) {
		c = get_char(1000);
		if (c == '+' || c == '-') {
			HAL_UART_Transmit(&COM_PORT, (uint8_t*)&c, 1, 10);
			HAL_UART_Transmit(&PC_PORT, (uint8_t*)&c, 1, 10);

			return c == '+' ? +1 : -1;
		}
		timeout--;
	}
	printf("input error\n\t");
	return -1;
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
		printf(" No.%02d [%+9ld,%+10ld] %4ld[dm]\t\n", i, goal.latitude, goal.longitude, goal.dist);
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
		HAL_Delay(10);
	}
}

void CUI_GoalList() {
	printf("\n\t\n\t ");
	printf("\n--------------------GNSS Goal Positions List--------------------\t\n");
	HAL_Delay(120);
	print_goalList();
	HAL_Delay(500);
	printf("\nType 0-9 to return to the first screen.\n\t");
	if (get_num(300) == 0) {
		return;
	}
}

void CUI_LogList() {
	printf("\n\t\n\t ");
	printf("\n--------------------Log Data List--------------------\t\n");
	HAL_Delay(120);
	print_logList();
	HAL_Delay(2000);
	printf("\nType 0-9 to return to the first screen.\n\t");
	if (get_num(300) == 0) {
		return;
	}
}

void CUI_SetGoal() {
	struct gnss_goal goal;
	int n = 0, s = 0;
	char c = '.';
	while (1) {
		printf("\n\n--------------------Set Goal--------------------\n\t");
		get_gnssGoal(&goal.latitude, &goal.longitude, &goal.dist);
		printf("\nCurrent goal position   [%+9ld,%+10ld] %ld[dm]\t\n", goal.latitude, goal.longitude, goal.dist);
		printf("\n1.Change and load preset.\t");
		printf("\n2.Load preset.\t");
		printf("\n0.Exit.\t\n");

		switch (get_num(300)) {

		case 1:
			printf("\n\nEnter preset number to change \ndd\t\n");
			n = get_num(300)*10;
			n += get_num(300);

			HAL_Delay(300);

			if (n > 15) {
				printf("Error\n");
				HAL_Delay(100);
				break;
			}

			printf("\n\nEnter latitude\n+dd.dddddd\t\n");
			s = get_sign(300);
			goal.latitude  = get_num(300);	goal.latitude *= 10;
			goal.latitude += get_num(300);	goal.latitude *= 10;
			HAL_UART_Transmit(&COM_PORT, (uint8_t*)&c, 1, 10);
			goal.latitude += get_num(300);	goal.latitude *= 10;
			goal.latitude += get_num(300);	goal.latitude *= 10;
			goal.latitude += get_num(300);	goal.latitude *= 10;
			goal.latitude += get_num(300);	goal.latitude *= 10;
			goal.latitude += get_num(300);	goal.latitude *= 10;
			goal.latitude += get_num(300);	goal.latitude *= s;
			HAL_Delay(300);

			printf("\n\nEnter longitude\n+ddd.dddddd\t\n");
			s = get_sign(300);
			goal.longitude  = get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= 10;
			HAL_UART_Transmit(&COM_PORT, (uint8_t*)& c, 1, 10);
			goal.longitude += get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= 10;
			goal.longitude += get_num(300);	goal.longitude *= s;
			HAL_Delay(300);

			printf("\n\nEnter goal radius\ndd.d[m]\t\n");
			goal.dist = get_num(300);	goal.dist *= 10;
			goal.dist += get_num(300);	goal.dist *= 10;
			HAL_UART_Transmit(&COM_PORT, (uint8_t*)& c, 1, 1);
			goal.dist += get_num(300);
			HAL_Delay(300);

			printf("\n\nNew position  No.%0d [%+9ld,%+10ld] %ld[dm]\t\n", n, goal.latitude, goal.longitude, goal.dist);
			HAL_Delay(400);
			printf("Do you want to write to EEPROM? \nYES:1 \nNO :2-9,0 \t\n");

			if (get_num(300) == 1) {
				save_goal(n, goal.latitude, goal.longitude, goal.dist);
			}
			printf("\n\nDo you want to set the goal to a new position? \nYES:1 \nNO :2-9,0 \t\n");
			if (get_num(300) == 1) {
				printf("\n\t\n");
				set_goalFromEEPROM(n);
			}
			HAL_Delay(500);
			break;

		case 2:
			printf("\n\nEnter preset number to load \ndd\t\n");
			n = get_num(300) * 10;
			n += get_num(300);

			HAL_Delay(300);

			if (n > 15) {
				printf("Error\n");
				HAL_Delay(100);
				break;
			}

			goal = read_goal(n);
			printf("\n\nNew goal position  No.%0d [%+9ld,%+10ld] %ld[dm]\t\n", n, goal.latitude, goal.longitude, goal.dist);
			printf("Do you want to set the goal to a new position? \nYES:1 \nNO :2-9,0 \t\n");
			if (get_num(300) == 1) {
				printf("\n\t\n");
				set_goalFromEEPROM(n);
			}
			HAL_Delay(500);
			break;

		case 0:
			printf("Exit\n");
			HAL_Delay(200);
			return;
			break;
		default:
			printf("Error\n");
			HAL_Delay(1000);
			return;
		}
	}
}

void CUI_TxLog() {
	int n;
	unsigned long start_address = 0;
	unsigned long stop_address = 0;
	unsigned long address;
	uint32_t buff[64];

	printf("\n\t\n\t");
	printf("\n--------------------TX Log--------------------\n\t");
	printf("\nEnter Log File number to load \n(001-743)\t\n");
	n = get_num(300);  n *= 10;
	n += get_num(300); n *= 10;
	n += get_num(300);
	if (n < 1 || n>743) {
		printf("Error\n");
		HAL_Delay(100);
		return;
	}
	HAL_Delay(300);

	eeprom_readWord(0x080800C0 + (n * 8) + 4, &start_address);
	eeprom_readWord(0x080800C0 + (n * 8) + 12, &stop_address);
	stop_address--;

	printf("\n\nRead Flash Page 0x%04lX to 0x%04lX\t\n", start_address, stop_address);
	if (start_address == 0xFFFFFFFF || start_address == 0x00000000) {
		printf("No such file\t\n");
		HAL_Delay(1000);
		return;
	}
	printf("\nType 1 to start transfer.\t\n");
	printf("Type 2-9,0 to Exit.\n\t");
	if (get_num(300) != 1) {
		printf("\n\t\n");
		return;
	}
	HAL_Delay(400);
	printf("\n\t\n\t\n");

	for (address = start_address; address <= stop_address; address++) {
		flash_read_page(address, (uint8_t*)&buff[0]);
		
		if (buff[63] != 0)break;
		
		for (int i = 0; i < 64; i++) {
			printf("%08lX ", buff[i]);
			if(i==19 || i== 39 || i== 63)printf("\t\n");
		}
		//HAL_Delay(50);

	}
	printf("\t\n");
	HAL_Delay(2000);
}

void CUI_Delete() {
	printf("\n\t\n\t");
	printf("\n--------------------Delete--------------------\n\t");
	printf("\nAll Log Data will be deleted.\t");
	printf("\nDo you really want to delete ?\t");
	printf("\nYES :1\t");
	printf("\nNO  :2-9,0\n\t");
	HAL_Delay(100);
	if (get_num(60) == 1) {
		delete_Log();
	}
}

void CUI_main() {
	while (1) {
		printf("\n\t\n\t");
		HAL_Delay(5);
		printf("\n--------------------CanSat Setting Mode--------------------\t\n");
		printf("\n1.Display a list of GNSS Goal Positions. \t");
		printf("\n2.Display a list of Log Data.\t");
		printf("\n3.Set goal position.\t");
		printf("\n4.Transfer log file.\t");
		printf("\n5.Delete Log.\t");
		printf("\n\n0.Exit setting mode\t\n");

		switch (get_num(60)) {
		case 1:
			CUI_GoalList();
			break;
		case 2:
			CUI_LogList();
			break;
		case 3:
			CUI_SetGoal();
			break;
		case 4:
			CUI_TxLog();
			break;
		case 5:
			CUI_Delete();
			break;
		case 0:
			printf("Exit\n");
			HAL_Delay(1000);
			return;
			break;
		default:
			printf("Error\n");
			HAL_Delay(1000);
			return;
		}
	}
}