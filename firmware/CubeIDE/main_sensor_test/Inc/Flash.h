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

#include "GNSS.h"

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

void eeprom_writeWord(uint32_t address, uint32_t value);
void eeprom_readWord(uint32_t address, uint32_t* value);


struct gnss_goal read_goal(int num);//EEPROM����S�[�����W��ǂݏo��
void save_goal(int num, const int32_t latitude, const int32_t longitude, const uint64_t dist); //EEPROM�ɃS�[�����W����������

unsigned long get_startAddress(const uint8_t hh, const uint8_t mm, const uint8_t ss);//EEPROM���珑�����݊J�n�A�h���X�̓ǂݏo��
void delete_Log();//���O����

#endif /* FLASH_H_ */
