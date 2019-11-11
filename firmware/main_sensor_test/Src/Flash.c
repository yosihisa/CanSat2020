/*
 * Flash.c
 *
 *  Created on: Oct 29, 2019
 *      Author: yosihisa
 */


#include "flash.h"

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
	HAL_SPI_Receive(&SPI_PORT, &rx_buff, 3, 10);
	HAL_GPIO_WritePin(FLASH_CS_Port, FLASH_CS_Pin, GPIO_PIN_SET);
	return (rx_buff[0] << 16) + (rx_buff[1] << 8) + rx_buff[2];
}
