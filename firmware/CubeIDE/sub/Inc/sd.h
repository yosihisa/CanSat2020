/*
 * sd.h
 *
 *  Created on: Nov 2, 2019
 *      Author: yosihisa
 */

#ifndef SD_H_
#define SD_H_

#include "main.h"
#include "fatfs.h"

#define MAX_DIR 9999

int sd_init();
int sd_writeLog(char data[],uint32_t len);
int sd_writeJpg(uint8_t data[], uint32_t size, uint32_t* file_name);


#endif /* SD_H_ */
