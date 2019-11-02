/*
 * sd.c
 *
 *  Created on: Nov 2, 2019
 *      Author: yosihisa
 */


#include "SD.h"
#include "stdio.h"

FATFS fs;
FRESULT res_fs;
DIR dir;

uint16_t	jpg_dir  = 0; //ディレクトリ番号
uint32_t	jpg_name = 0; //ファイル番号
FIL			jpg_fil;
FIL			lof_fil;

int sd_init() {

	//マウント
	res_fs = f_mount(&fs, "", 1);
	if (res_fs != FR_OK) {
		printf("f_mount() : error %u\n", res_fs);
		return res_fs;
	}

	res_fs = f_opendir(&dir, "");
	if (res_fs != FR_OK) {
		printf("f_opendir() : error %u\n", res_fs);
		return res_fs;
	}
	printf("Init SDcard\n");

	//ディレクトリの作成
	char path[30];
	for (jpg_dir = 0; jpg_dir <= MAX_DIR; jpg_dir++) {
		
		if (jpg_dir == MAX_DIR) {
			printf("Capacity shortage\n");
			return -1;
		}

		sprintf(path, "/%05d", jpg_dir);
		res_fs = f_mkdir(path);
		if (res_fs == FR_OK) {
			break;
		}
		else {
			printf("f_mkdir:error %s %u\n", path, res_fs);
		}
	}

	jpg_name = 0;

	//制御履歴ファイルの作成
	sprintf(path, "/%05d/log.csv", jpg_dir);
	res_fs = f_open(&lof_fil, (char*)path, FA_WRITE | FA_CREATE_ALWAYS);
	if (res_fs != FR_OK) {
		printf("f_open() : error %u\n", res_fs);
		return res_fs;
	}
	res_fs = f_sync(&lof_fil);
	if (res_fs != FR_OK) {
		printf("f_sync() : error %u\n", res_fs);
		return res_fs;
	}

	printf("logfile = %s \n", path);

	return 0;
}

int sd_writeLog(char data[], uint32_t len) {
	unsigned int writeBytes;
	res_fs = f_write(&lof_fil, data, len, &writeBytes);
	f_sync(&lof_fil);
	if (res_fs != FR_OK) {
		printf("sd_writeLog() : error %u\n", res_fs);
		return res_fs;
	}
	return res_fs;
}

int sd_writeJpg(uint8_t data[], uint32_t size, uint32_t* file_name) {
	char path[30];
	unsigned int writeBytes;

	sprintf(path, "/%05d/%05ld.jpg", jpg_dir, jpg_name);

	f_open(&jpg_fil, path, FA_WRITE | FA_CREATE_ALWAYS);
	res_fs = f_write(&jpg_fil, data, size, &writeBytes);
	f_close(&jpg_fil);

	*file_name = jpg_name;
	jpg_name++;

	if (res_fs != FR_OK) {
		printf("sd_writeJpg() : error %u\n", res_fs);
		return res_fs;
	}

	return FR_OK;
}
