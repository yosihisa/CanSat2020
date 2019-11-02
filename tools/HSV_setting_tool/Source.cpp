#include "DxLib.h"
#include <stdio.h>
#include <string.h>
#include "tjpgd.h"
#include <Windows.h>

typedef struct {
	FILE *fp;
} IODEV;


//�F���͈̔�
#define H_MIN_1 0	//�Œ�
#define H_MAX_1 10

#define H_MIN_2 330	
#define H_MAX_2 360 //�Œ�

//�ʓx�͈̔�
#define S_MIN 45  * 2.56
#define S_MAX 100 * 2.56

//���x�͈̔�
#define V_MIN 30  * 2.56
#define V_MAX 100 * 2.56

#define FILENAME "IMG\\%03d.jpg"
#define HEIGHT 480
#define WIDTH  640
BYTE RED_bool[HEIGHT][WIDTH / 8];



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
	short h, s, v; //H:�F�� S:�ʓx V:���x

	src = (BYTE*)bitmap;
	for (height = 0; height < (rect->bottom - rect->top + 1); height++) {
		for (width = 0; width < (rect->right - rect->left + 1); width++) {

			//�o�b�t�@��RGB88����e�F�𒊏o
			r = (BYTE)*(src + 3 * (height*(rect->right - rect->left + 1) + width));
			g = (BYTE)*(src + 3 * (height*(rect->right - rect->left + 1) + width) + 1);
			b = (BYTE)*(src + 3 * (height*(rect->right - rect->left + 1) + width) + 2);

			//HSV�ϊ�
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

			if ((h >= H_MIN_1 && h <= H_MAX_1) || (h >= H_MIN_2 && h <= H_MAX_2)) {
				if ((s >= S_MIN && s <= S_MAX) && (v >= V_MIN && v <= V_MAX)) {
					bitshift = (rect->left + width) % 8;
					RED_bool[rect->top + height][(rect->left + width) / 8] |= (0b10000000 >> bitshift);

				}
			}
		}
	}

	return 1;
}


// �v���O������ WinMain ����n�܂�܂�
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	ChangeWindowMode(TRUE);
	SetGraphMode(400, 480, 32);


	if (DxLib_Init() == -1)		// �c�w���C�u��������������
	{
		return -1;			// �G���[���N�����璼���ɏI��
	}
	SetDrawScreen(DX_SCREEN_BACK);
	SetWindowUserCloseEnableFlag(FALSE);

	JDEC jdec;        /* Decompression object */
	JRESULT res;      /* Result code of TJpgDec API */
	IODEV devid;      /* User defined device identifier */
	BYTE work[3100];
	int filenum = 0;

	char str[256],path[256];
	int flag = 0;
	while (ScreenFlip() == 0 && ProcessMessage() == 0) {
		if (flag == 0) {
			ClearDrawScreen();
			sprintf_s(path, FILENAME, filenum);
			fopen_s(&devid.fp, path, "rb");

			//�f�R�[�h����
			res = jd_prepare(&jdec, in_func, work, 3100, &devid);
			memset(RED_bool, 0, sizeof(RED_bool));

			if (res == JDR_OK) {
				//�f�R�[�h�J�n
				res = jd_decomp(&jdec, out_func, 0);
				if (res == JDR_OK) {

					UINT xc = 0, yc = 0, s = 0;

					//�ϊ���̉摜�`��
					for (UINT h = 0; h < jdec.height; h++) {
						for (UINT w = 0; w < jdec.width; w++) {
							if ((RED_bool[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
								DrawPixel(w, h, GetColor(255, 0, 0));	// �_��ł�
							}
						}
					}

					//�d�S�v�Z
					for (UINT h = 0; h < jdec.height; h++) {
						for (UINT w = 0; w < jdec.width; w++) {
							if ((RED_bool[h][w / 8] & (0b10000000 >> (w % 8))) != 0) {
								xc += w;
								yc += h;
								s++;
							}
						}
					}
					if (s != 0) {
						xc = xc / s;
						yc = yc / s;
						sprintf_s(str, "%03d.jpg (x,y) = (%d,%d) ,s = %d\n", filenum, xc, yc, s);
						SetMainWindowText(str);

						DrawCircle(xc, yc, 3, GetColor(0, 255, 255), FALSE);

					} else {
						//printfDx("(x,y) = Null ");
						sprintf_s(str, "%03d.jpg (x,y) = Null", filenum);
						SetMainWindowText(str);

					}
				} else {
					printfDx("Failed to decompress: rc=%d\n", res);
				}


			} else {
				printfDx("Failed to prepare: rc=%d\n", res);
			}
			fclose(devid.fp);

			LoadGraphScreen(0, 241, path, FALSE);
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

	DxLib_End();				// �c�w���C�u�����g�p�̏I������

	return 0;				// �\�t�g�̏I�� 
}