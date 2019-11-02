/**
 * @file C1098.h
 * @author @n_yosihisa
 * @date 2019/02/11
 */

#include "stm32f2xx_hal.h"

#define CAMERA_PORT huart6
typedef enum {
	CAMERA_OK = 0,				  /* (0) Succeeded */
	CAMERA_ERR,			          /* (1) エラー */
	CAMERA_COM_ERR,               /* (2) 通信エラー */
	CAMERA_NOT_ENOUGH_CORE,       /* (3)メモリ不足 */
	CAMERA_FAILED_CHANGE_BAUDRATE /* (4)ボーレート変更に失敗 */

} CAMERARESULT;

typedef enum {
	QVGA = 0,
	VGA
} CAMERA_RESOLUTION;

typedef struct {
	UART_HandleTypeDef *uart_port;
	uint32_t packet_size;
	uint32_t baudrate;
	CAMERA_RESOLUTION resolution;
} c1098_handle;

CAMERARESULT init_C1098(); //初期化
CAMERARESULT snapShot();//写真を撮る
CAMERARESULT getPicture(uint8_t *buffer,uint32_t size , uint32_t *data_size);//写真を転送する

//CAMERARESULT get_picture_size(c1098_handle* handle, uint32_t* data_size);//写真のサイズを得る
//CAMERARESULT get_picture_packet(c1098_handle* handle, uint8_t data[], uint16_t id);//写真をパケット単位で転送する

