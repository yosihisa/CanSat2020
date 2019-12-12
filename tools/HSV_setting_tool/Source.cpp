#include "DxLib.h"
#include <stdio.h>
#include <string.h>
#include "tjpgd.h"
#include <Windows.h>

typedef struct {
	FILE *fp;
} IODEV;



//------ゴール-----
//色相の範囲
#define G_H_MIN_1 0
#define G_H_MAX_1 10

#define G_H_MIN_2 330
#define G_H_MAX_2 360

//彩度の範囲
#define G_S_MIN 115
#define G_S_MAX 256

//明度の範囲
#define G_V_MIN 77
#define G_V_MAX 256

//------パラシュート-----
//色相の範囲
#define P_H_MIN_1 300	
#define P_H_MAX_1 300

#define P_H_MIN_2 280
#define P_H_MAX_2 310

//彩度の範囲
#define P_S_MIN 50
#define P_S_MAX 180

//明度の範囲
#define P_V_MIN 50
#define P_V_MAX 200

#define FILENAME "IMG\\%05d.jpg"
#define HEIGHT 480
#define WIDTH  640
BYTE RED_bool_G[HEIGHT][WIDTH / 8];
BYTE RED_bool_P[HEIGHT][WIDTH / 8];



UINT in_func(JDEC* jd, BYTE* buff, UINT nbyte){
	IODEV *dev = (IODEV*)jd->device;
	if (buff) {
		return (UINT)fread(buff, 1, nbyte, dev->fp);
	} else {
		return fseek(dev->fp, nbyte, SEEK_CUR) ? 0 : nbyte;
	}
}


UINT out_func(JDEC* jd, void* bitmap, JRECT* rect)
{

	BYTE *src;
	int width, height, bitshift;
	BYTE r, g, b;
	short h, s, v; //H:色相 S:彩度 V:明度

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
				if (MAX == r) h = 60.0*(g - b) / (MAX - MIN) + 0;
				else if (MAX == g) h = 60.0*(b - r) / (MAX - MIN) + 120.0;
				else if (MAX == b) h = 60.0*(r - g) / (MAX - MIN) + 240.0;

				if (h > 360.0) h = h - 360.0;
				else if (h < 0) h = h + 360.0;
				s = (MAX - MIN) * 256 / MAX ;
			}

			if (h > 360.0)h -= 360;

			if ((h >= G_H_MIN_1 && h <= G_H_MAX_1) || (h >= G_H_MIN_2 && h <= G_H_MAX_2)) {
				if ((s >= G_S_MIN && s <= G_S_MAX) && (v >= G_V_MIN && v <= G_V_MAX)) {
					bitshift = (rect->left + width) % 8;
					RED_bool_G[rect->top + height][(rect->left + width) / 8] |= (0b10000000 >> bitshift);

				}
			
			}
			if ((h >= P_H_MIN_1 && h <= P_H_MAX_1) || (h >= P_H_MIN_2 && h <= P_H_MAX_2)) {
				if ((s >= P_S_MIN && s <= P_S_MAX) && (v >= P_V_MIN && v <= P_V_MAX)) {
					bitshift = (rect->left + width) % 8;
					RED_bool_P[rect->top + height][(rect->left + width) / 8] |= (0b10000000 >> bitshift);

				}
			}
		}
	}

	return 1;
}


// プログラムは WinMain から始まります
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	ChangeWindowMode(TRUE);
	SetGraphMode(640*2, 480*2, 32);


	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}
	SetDrawScreen(DX_SCREEN_BACK);
	SetWindowUserCloseEnableFlag(FALSE);

	JDEC jdec;        /* Decompression object */
	JRESULT res;      /* Result code of TJpgDec API */
	IODEV devid;      /* User defined device identifier */
	BYTE work[3100];
	int filenum = 0;

	unsigned int Color = GetColor(255, 255, 255);

	char str[256],path[256];
	int flag = 0;
	while (ScreenFlip() == 0 && ProcessMessage() == 0) {
		if (flag == 0) {
			ClearDrawScreen();
			sprintf_s(path, FILENAME, filenum);
			fopen_s(&devid.fp, path, "rb");

			//デコード準備
			res = jd_prepare(&jdec, in_func, work, 3100, &devid);
			memset(RED_bool_G, 0, sizeof(RED_bool_G));
			memset(RED_bool_P, 0, sizeof(RED_bool_P));


			if (res == JDR_OK) {
				//デコード開始
				res = jd_decomp(&jdec, out_func, 0);
				if (res == JDR_OK) {

					UINT xc_G = 0, yc_G = 0, s_G = 0;
					UINT xc_P = 0, yc_P = 0, s_P = 0;

					//変換後の画像描画
					for (UINT h = 0; h < jdec.height; h++) {
						for (UINT w = 0; w < jdec.width; w++) {
							if ((RED_bool_G[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
								DrawPixel(w+641, h, GetColor(255, 0, 0));	// 点を打つ
							}
						}
					}

					for (UINT h = 0; h < jdec.height; h++) {
						for (UINT w = 0; w < jdec.width; w++) {
							if ((RED_bool_P[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
								DrawPixel(w + 641, h+481, GetColor(255, 0, 255));	// 点を打つ
							}
						}
					}

					//重心計算
					for (UINT h = 0; h < jdec.height; h++) {
						for (UINT w = 0; w < jdec.width; w++) {
							if ((RED_bool_G[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
								xc_G += w;
								yc_G += h;
								s_G++;
							}
						}
					}
					if (s_G != 0) {
						xc_G = xc_G / s_G;
						yc_G = yc_G / s_G;
					}
					for (UINT h = 0; h < jdec.height; h++) {
						for (UINT w = 0; w < jdec.width; w++) {
							if ((RED_bool_P[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
								xc_P += w;
								yc_P += h;
								s_P++;
							}
						}
					}
					if (s_P != 0) {
						xc_P = xc_P / s_P;
						yc_P = yc_P / s_P;
					}
					//計算結果の表示
					DrawFormatString(10, 500, Color,"%s", path);
					DrawFormatString(10, 530, Color, "G(x,y) = (%3d,%3d) ,s = %5d\n", xc_G, yc_G, s_G);
					DrawFormatString(10, 550, Color, "P(x,y) = (%3d,%3d) ,s = %5d\n", xc_P, yc_P, s_P);

					DrawCircle(xc_G+641, yc_G, 3, GetColor(0, 255, 255), FALSE);
					DrawCircle(xc_P+641, yc_P+481, 3, GetColor(0, 255, 255), FALSE);

				} else {
					printfDx("Failed to decompress: rc=%d\n", res);
				}


			} else {
				printfDx("Failed to prepare: rc=%d\n", res);
			}
			fclose(devid.fp);

			LoadGraphScreen(0, 0, path, FALSE);
			DrawLine(0, 480, 640 * 2, 480, GetColor(255, 255, 255));
			DrawLine(640, 0, 640, 480*2, GetColor(255, 255, 255));
			flag = 1;
		}
		if (GetWindowUserCloseFlag(TRUE)) {
			break;
		}
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) {
			break;
		}
		if (CheckHitKey(KEY_INPUT_RIGHT) == 1) {
			flag = 0;
			filenum++;
			sprintf_s(path, FILENAME, filenum);

			int error  =fopen_s(&devid.fp, path, "rb");
			if (error != 0) {
				flag = 1;
				filenum--;
			}
			else fclose(devid.fp);
		}
		if (CheckHitKey(KEY_INPUT_LEFT) == 1) {
			flag = 0;
			filenum--;
			sprintf_s(path, FILENAME, filenum);

			int error = fopen_s(&devid.fp, path, "rb");
			if (error != 0) {
				flag = 1;
				filenum++;
			}
			else fclose(devid.fp);
		}
		
		WaitTimer(50);
	}

	DxLib_End();				// ＤＸライブラリ使用の終了処理

	return 0;				// ソフトの終了 
}