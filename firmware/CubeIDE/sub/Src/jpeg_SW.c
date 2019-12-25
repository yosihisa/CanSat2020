#include "jpeg_SW.h"
#include <stdio.h>
#include <string.h>

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

short H_MIN_1 = 0;
short H_MAX_1 = 0;
short H_MIN_2 = 0;
short H_MAX_2 = 0;
short S_MIN = 0;
short S_MAX = 0;
short V_MIN = 0;
short V_MAX = 0;

//ターゲットカラーの変更
void changeMode(GPIO_PinState mode) {
	if (mode == GOAL) {
		H_MIN_1 = G_H_MIN_1;
		H_MAX_1 = G_H_MAX_1;
		H_MIN_2 = G_H_MIN_2;
		H_MAX_2 = G_H_MAX_2;
		S_MIN   = G_S_MIN  ;
		S_MAX   = G_S_MAX  ;
		V_MIN   = G_V_MIN  ;
		V_MAX   = G_V_MAX  ;
	}
	else {
		H_MIN_1 = P_H_MIN_1;
		H_MAX_1 = P_H_MAX_1;
		H_MIN_2 = P_H_MIN_2;
		H_MAX_2 = P_H_MAX_2;
		S_MIN   = P_S_MIN  ;
		S_MAX   = P_S_MAX  ;
		V_MIN   = P_V_MIN  ;
		V_MAX   = P_V_MAX  ;
	}
}

//JPEGデコーダ入力関数
UINT in_func(JDEC* jd, BYTE* buff, UINT nbyte){
	IODEV *dev = (IODEV*)jd->device;   
	if (buff) {
		uint32_t e;
		if (dev->seek+ nbyte <= dev->size) {
			e = nbyte;
		} else {
			e = dev->seek+ nbyte - dev->size;
		}
		for (int i = 0; i < e; i++) {
			*(buff + i) = dev->data[dev->seek + i];
		}
		dev->seek += e;
		
		return e;

	} else {

		if (dev->seek + nbyte <= dev->size) {
			dev->seek += nbyte;
			return nbyte;

		} else {
			return 0;
		}
	}
}

//JPEGデコーダ出力関数
UINT out_func(JDEC* jd, void* bitmap, JRECT* rect)
{
    IODEV *dev = (IODEV*)jd->device;       
	uint8_t *src;
	uint32_t width, height,bitshift;
	uint8_t r, g, b;
	short h=0, s, v; //H:色相 S:彩度 V:明度

	src = (BYTE*)bitmap;
	for (height = 0; height < (rect->bottom - rect->top + 1); height++) {
		for (width = 0; width < (rect->right - rect->left + 1); width++) {

			//バッファのRGB88から各色を抽出
			r = (BYTE)*(src + 3 * (height*(rect->right - rect->left + 1) + width));
			g = (BYTE)*(src + 3 * (height*(rect->right - rect->left + 1) + width) + 1);
			b = (BYTE)*(src + 3 * (height*(rect->right - rect->left + 1) + width) + 2);

			//HSV変換
			BYTE MAX = max((max(r, g)), b);
			BYTE MIN = min((min(r, g)), b);
			v = MAX;

			if (MAX == MIN) {
				h = 0;
				s = 0;
			} else {
				if (MAX == r) h = 60*(g - b) / (MAX - MIN) + 0;
				else if (MAX == g) h = 60*(b - r) / (MAX - MIN) + 120;
				else if (MAX == b) h = 60*(r - g) / (MAX - MIN) + 240;

				if (h > 360) h = h - 360;
				else if (h < 0) h = h + 360;
				s = (MAX - MIN) * 256 / MAX;

			}

			if (h > 360)h -= 360;

			if ((h >= H_MIN_1 && h <= H_MAX_1)|| (h >= H_MIN_2 && h <= H_MAX_2)) {
				if ((s >= S_MIN && s <= S_MAX) && (v >= V_MIN && v <= V_MAX)) {
					bitshift = (rect->left + width) % 8;
					dev->RED_bool[rect->top + height][(rect->left + width) / 8] |= (0b10000000 >> bitshift);

				}
			}
		}
	}
	return 1;
}

//JPEG decode
int decode(IODEV* img) {

	JRESULT jpeg_res;
	JDEC jdec;
	uint8_t work[3100];

	img->xc = 0;
	img->yc = 0;
	img->s = 0;
	img->seek = 0;
	memset(&img->RED_bool, 0, HEIGHT * WIDTH / 8);

	jpeg_res = jd_prepare(&jdec, in_func, work, 3100, &img->data);
	if (jpeg_res != JDR_OK) {
		printf("jd_prepare() error: %u\n", jpeg_res);
		return jpeg_res;
	}

	//デコード開始
	jpeg_res = jd_decomp(&jdec, out_func, 0);
	if (jpeg_res != JDR_OK) {
		printf("jd_decomp() error: %u\n", jpeg_res);
		return jpeg_res;
	}

	//重心計算
	for (UINT h = 0; h < jdec.height; h++) {
		for (UINT w = 0; w < jdec.width; w++) {
			if ((img->RED_bool[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
				img->xc += w;
				img->yc += h;
				img->s++;
			}
		}
	}
	if (img->s != 0) {
		img->xc /= img->s;
		img->yc /= img->s;
	}
	else {
		img->xc = 0;
		img->yc = 0;
	}

	return 0;
}

