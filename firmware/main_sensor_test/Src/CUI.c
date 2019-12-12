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
