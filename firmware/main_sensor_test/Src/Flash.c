/*
 * Flash.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */


#include "flash.h"
#include <stdio.h>

#define SPI_PORT hspi2
extern SPI_HandleTypeDef SPI_PORT;
#define FLASH_CS_Pin  SPI2_CS_Pin
#define FLASH_CS_Port SPI2_CS_GPIO_Port


void write_enable() {
	uint8_t tx_buff = 0x06;
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, &tx_buff, 1, 10);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

void write_disable() {
	uint8_t tx_buff = 0x04;
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, &tx_buff, 1, 10);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

uint8_t flash_read_state() {
	uint8_t tx_buff = 0x05;
	uint8_t rx_buff;

	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, &tx_buff, 1, 10);
	HAL_SPI_Receive(&SPI_PORT, &rx_buff, 1, 10);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	return rx_buff;
}

int flash_read_page(uint16_t page, uint8_t data[]) {
	uint8_t tx_buff[4];

	//pageが範囲内であることを確認
	if (0xFFFF < page) {
		return 1;
	}

	tx_buff[1] = page >> 8;
	tx_buff[2] = page >> 0;
	tx_buff[3] = 0;

	tx_buff[0] = 0x03;
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, tx_buff, 4, 10);
	HAL_SPI_Receive(&SPI_PORT, data, 256, 100);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);

	return 0;
}

int flash_write_page(uint16_t page, uint8_t data[]) {
	uint8_t tx_buff[4];
	uint8_t state;

	//pageが範囲内であることを確認
	if (0xFFFF < page) {
		return 1;
	}

	tx_buff[1] = page >> 8;
	tx_buff[2] = page >> 0;
	tx_buff[3] = 0;

	do {
		write_enable();
		state = flash_read_state();
	} while ((state & 0b00000010) != 0b00000010);

	//書き込み
	tx_buff[0] = 0x02;
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, tx_buff, 4, 10);
	HAL_SPI_Transmit(&SPI_PORT, data, 256, 100);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);

	do {
		state = flash_read_state();
	} while ((state & 0b00000001) == 0b00000001);

	do {
		write_disable();
		state = flash_read_state();
	} while ((state & 0b00000010) == 0b00000010);


	return 0;
}

int flash_erase_64k(uint8_t sector)
{
	uint8_t tx_buff[4];
	uint8_t state;

	//pageが範囲内であることを確認
	if (0xFF < sector) {
		return 1;
	}

	tx_buff[1] = sector;
	tx_buff[2] = 0;
	tx_buff[3] = 0;

	do {
		write_enable();
		state = flash_read_state();
	} while ((state & 0b00000010) != 0b00000010);

	//消去
	tx_buff[0] = 0xD8;
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, tx_buff, 4, 10);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);

	do {
		state = flash_read_state();
	} while ((state & 0b00000001) == 0b00000001);

	do {
		write_disable();
		state = flash_read_state();
	} while ((state & 0b00000010) == 0b00000010);


	return 0;
}

uint32_t flash_read_ID() {
	uint8_t tx_buff = 0x9F;
	uint8_t rx_buff[3];

	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&SPI_PORT, &tx_buff, 1, 10);
	HAL_SPI_Receive(&SPI_PORT, rx_buff, 3, 10);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	return (rx_buff[0] << 16) + (rx_buff[1] << 8) + rx_buff[2];
}

void eeprom_writeWord(uint32_t address, uint32_t value) {
	if (!IS_FLASH_DATA_ADDRESS(address)) {
		printf("eeprom_writeWord() error.");
		return;
	}
	HAL_FLASHEx_DATAEEPROM_Unlock();
	HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, address, value);
	HAL_FLASHEx_DATAEEPROM_Lock();
}

void eeprom_readWord(uint32_t address, uint32_t* value) {
	if (!IS_FLASH_DATA_ADDRESS(address)) {
		printf("eeprom_readWord() error.");
		return;
	}
	*value = *(__I uint32_t*)address;
}

struct gnss_goal read_goal(int num) {
	struct gnss_goal data = { 0 };
	if (num < 0 || num>15) {
		printf("read_goal() error.");
		return data;
	}
	eeprom_readWord(DATA_EEPROM_BASE + (num * 12) + 0, (uint32_t*)&data.latitude);
	eeprom_readWord(DATA_EEPROM_BASE + (num * 12) + 4, (uint32_t*)&data.longitude);
	eeprom_readWord(DATA_EEPROM_BASE + (num * 12) + 8, (uint32_t*)&data.dist);
	return data;
}

void save_goal(int num, const int32_t latitude, const int32_t longitude, const uint64_t dist) {
	if (num < 0 || num>15) {
		printf("read_goal() error.");
		return;
	}
	eeprom_writeWord(DATA_EEPROM_BASE + (num * 12) + 0, latitude);
	eeprom_writeWord(DATA_EEPROM_BASE + (num * 12) + 4, longitude);
	eeprom_writeWord(DATA_EEPROM_BASE + (num * 12) + 8, dist);
}

unsigned long get_startAddress(const uint8_t hh, const uint8_t mm, const uint8_t ss) {
	unsigned long time, flash_address=0;
	unsigned long eeprom_address;
	uint8_t data[256];

	for (eeprom_address = 0x08080000 + 0xC0; eeprom_address < 0x08080000 + 0x17F0 ; eeprom_address+=8) {
		eeprom_readWord(eeprom_address, &time);
		
		if (time == 0xFFFFFFFF) {
		
			if (eeprom_address == 0x080800C0) {
				flash_address = 0;
			}
			else {
				eeprom_readWord(eeprom_address - 4, &flash_address);
			}

			break;
		}

	}


	while (flash_address < 0xFFFF) {
		flash_read_page(flash_address, data);
		if (data[255] ==0xFF )break;
		flash_address++;
	}

	time = (hh << 16) + (mm << 8) + ss;
	eeprom_writeWord(eeprom_address + 0, time);
	eeprom_writeWord(eeprom_address + 4, flash_address);

	return flash_address;
}

void delete_Log() {
	/*printf("Start clear_Log \t\n");
	for (int i = 0; i <= 0xFF; i++) {
		flash_erase_64k(i);
		printf("Delete flash Sector %02X\t\n", i);
	}*/
	printf("Delete EEPROM \t\n");
	for (unsigned long i = 0x08080000 + 0xC0; i < 0x08080000 + 0x17FF; i+=4) {
		eeprom_writeWord(i, 0xFFFFFFFF);
	}
	printf("Done clear_Log \t\n");
}
