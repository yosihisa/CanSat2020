#include "stm32f2xx_hal.h"
#include "tjpgd.h"

#include "_Setting.h"

#define GOAL GPIO_PIN_RESET
#define PARA GPIO_PIN_SET

uint8_t RED_bool[HEIGHT][WIDTH/8];

#define MAX_SIZE 40*1024

typedef struct {
	uint8_t data[MAX_SIZE];
	uint32_t seek ;
    uint32_t size ;
    uint8_t RED_bool[HEIGHT][WIDTH / 8];
	
	uint32_t xc, yc, s;
} IODEV;

UINT in_func(JDEC* jd, BYTE* buff, UINT nbyte);
UINT out_func(JDEC* jd, void* bitmap, JRECT* rect);

void changeMode(GPIO_PinState mode);

int decode(IODEV* img);
