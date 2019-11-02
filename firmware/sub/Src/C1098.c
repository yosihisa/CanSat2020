/**
 * @file C1098.c
 * @author @n_yosihisa
 * @date 2019/02/11
 */

#include "C1098.h"
#include "_Setting.h"


extern UART_HandleTypeDef CAMERA_PORT;
c1098_handle camera = {
	.uart_port = &CAMERA_PORT,
	.packet_size = 256,
	.baudrate = 460800,
	.resolution = RESOLUTION
};

//通信速度の変更
HAL_StatusTypeDef Change_UART_BaudRate(UART_HandleTypeDef *huart ,uint32_t baudrate){
	huart->Init.BaudRate = baudrate;
	return HAL_UART_Init(huart);
}

//syncコマンドの送信
CAMERARESULT sync(c1098_handle *handle){
	uint8_t sync[6] ={0xAA,0x0D,0x00,0x00,0x00,0x00};
	uint8_t ack[6] = {0xAA,0x0E,0x00,0x00,0x00,0x00};
	uint8_t c_data[12];

	HAL_UART_Transmit( handle->uart_port, sync, 6, 6 );
	HAL_UART_Receive( handle->uart_port, c_data, 12, 100);
	if (c_data[0]==0xAA && c_data[1]==0x0E && c_data[2]==0x0D  && c_data[6]==0xAA && c_data[7]==0x0D && c_data[8]==0x00){
		HAL_UART_Transmit( handle->uart_port, ack, 6, 6 );
		return CAMERA_OK;
	}
	return CAMERA_COM_ERR;
}

//カメラと通信できるようにする
CAMERARESULT c1098_sync(c1098_handle *handle){

	//ホットスタート
	if (Change_UART_BaudRate(handle->uart_port,handle->baudrate)!=HAL_OK){
		return CAMERA_FAILED_CHANGE_BAUDRATE;
	}
	if (sync(handle) == CAMERA_OK){
		return CAMERA_OK;
	}

	//応答が無かったらデフォルトの速度で通信
	if (Change_UART_BaudRate(handle->uart_port,14400)!=HAL_OK){
		return CAMERA_FAILED_CHANGE_BAUDRATE;
	}

	//60回syncを送出
	for (int i =0 ; i<60 ; i ++ ){
		if (sync(handle) == CAMERA_OK){
			return CAMERA_OK;
		}
	}
	return CAMERA_COM_ERR;
}

//カメラの初期設定
CAMERARESULT c1098_init(c1098_handle *handle){
	uint8_t initial[6] = {0xAA,0x01,0x07,0x07,0x00,0x00};
	uint8_t set_package_size[6] = {0xAA,0x06,0x08,0x00,0x01,0x00};

	uint8_t c_data[12];

	if (handle->resolution == QVGA)initial[5]=0x05;
	if (handle->resolution ==  VGA)initial[5]=0x07;

	set_package_size[3] = (handle->packet_size) ;
	set_package_size[4] = (handle->packet_size) >>8;

	if (handle->baudrate == 14400)initial[2]=0x07;
	if (handle->baudrate == 28800)initial[2]=0x06;
	if (handle->baudrate == 57600)initial[2]=0x05;
	if (handle->baudrate == 115200)initial[2]=0x04;
	if (handle->baudrate == 230400)initial[2]=0x03;
	if (handle->baudrate == 460800)initial[2]=0x02;

	//sync
	if (c1098_sync(handle) != CAMERA_OK){
		return CAMERA_COM_ERR;
	}

	//Cahge BaudRate
	HAL_UART_Transmit( handle->uart_port, initial, 6, 6 );
	HAL_UART_Receive(handle->uart_port, c_data, 6, 100);
	if (c_data[0]==0xAA && c_data[1]==0x0E && c_data[2]==0x01){

		if (Change_UART_BaudRate(handle->uart_port,handle->baudrate) != HAL_OK){
			return CAMERA_FAILED_CHANGE_BAUDRATE;
		}
	}else{
		return CAMERA_ERR;
	}

	HAL_Delay(60);

	//Cange Package Size
	HAL_UART_Transmit(handle->uart_port, set_package_size, 6, 6 );
	HAL_UART_Receive(handle->uart_port, c_data, 6, 100);
	if (c_data[0]==0xAA && c_data[1]==0x0E && c_data[2]==0x06){
		return CAMERA_OK;
	}else{
		return CAMERA_ERR;
	}
}

//写真を撮影する
CAMERARESULT snap_shot(c1098_handle *handle){
	uint8_t c_data[6];
	uint8_t snap_shot[6] = {0xAA,0x05,0x00,0x00,0x00,0x00};
	HAL_UART_Transmit(handle->uart_port, snap_shot, 6, 6 );
	HAL_UART_Receive(handle->uart_port, c_data, 6, 100);
	if (c_data[0]==0xAA && c_data[1]==0x0E && c_data[2]==0x05){
		return CAMERA_OK;
	}else{
		return CAMERA_ERR;
	}
}

//撮影した写真を転送する
CAMERARESULT get_picture(c1098_handle *handle,uint8_t *buffer,uint32_t buffer_size , uint32_t *received_size){
	uint8_t get_picture[6] = {0xAA,0x04,0x01,0x00,0x00,0x00};
	uint8_t  c_data[6];
	uint32_t data_length =0;
	uint16_t packet_size =0;
	uint16_t id =0;

	*received_size = 0;

	//Transmit GET PICTURE
	HAL_UART_Transmit(handle->uart_port, get_picture, 6, 6 );

	//Receive ACK
	HAL_UART_Receive(handle->uart_port, c_data, 6, 100);
	if (c_data[0]!=0xAA || c_data[1]!=0x0E || c_data[2]!=0x04){
		return CAMERA_ERR;
	}

	//Receive Data Length
	HAL_UART_Receive(handle->uart_port, c_data, 6, 1000);
	if (c_data[0]!=0xAA || c_data[1]!=0x0A || c_data[2]!=0x01){
		return CAMERA_ERR;
	}

	//Data Length
	data_length |= c_data[5]<<16;
	data_length |= c_data[4]<<8;
	data_length |= c_data[3];


	//バッファの容量が足りているか確認
	if ( data_length >= buffer_size ){
		return CAMERA_NOT_ENOUGH_CORE;
	}

	//受信
	uint8_t ack[6] = {0xAA,0x0E,0x00,0x00,0x00,0x00};
	while(1){
		//ACK with Package ID
		ack[4]= (uint8_t)  id;
		ack[5]= (uint8_t) (id>>8);
		HAL_UART_Transmit(handle->uart_port, ack, 6, 6 );

		//Image Data Package
		HAL_UART_Receive(handle->uart_port, c_data, 4, 100);
		packet_size  = c_data[3]<<8;
		packet_size |= c_data[2];
		HAL_UART_Receive(handle->uart_port, buffer + (handle->packet_size-6)*id, packet_size, packet_size + 100);
		HAL_UART_Receive(handle->uart_port, c_data, 2, 100);

		*received_size += packet_size;
		if ( *received_size ==  data_length ){
			break;
		}
		id++;
	}
	ack[4] = 0xF0;
	ack[5] = 0xF0;
	HAL_UART_Transmit(handle->uart_port, get_picture, 6, 6 );

	return CAMERA_OK;
}

CAMERARESULT init_C1098() {
	return c1098_init(&camera);
}

CAMERARESULT snapShot() {
	return snap_shot(&camera);
}

CAMERARESULT getPicture(uint8_t* buffer, uint32_t size, uint32_t* data_size) {
	return get_picture(&camera, buffer, size, data_size);
}
