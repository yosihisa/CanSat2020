/*
 * Flash.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "stm32l0xx_hal.h"
#include "main.h"

#define BLOCK_MAX 0xFF

//void write_enable(FLASH_SELECTOR flash_selector);
//void write_disable(FLASH_SELECTOR flash_selector);

uint8_t flash_read_state();
int flash_read_page   (uint16_t page, uint8_t data[]);
int flash_write_page  (uint16_t page, uint8_t data[]);
//int flash_erase_4k(uint16_t sector);
//int flash_erase_32k(uint16_t sector);
int flash_erase_64k(uint8_t sector);

uint32_t flash_read_ID();


#endif /* FLASH_H_ */
