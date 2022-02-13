/*
 * enums_structs.h
 *
 *  Created on: Dec 30, 2021
 *      Author: Mecit
 */

#pragma once

#include "stm32g4xx_hal.h"
#include "datapacket.h"


// for now
#include "flagControl.h"

#define RESET_SENSOR_FLAGS ( SENSOR ) ( SENSOR.RESET_FLAG = 0; )
//////////////////////////////// ENUM PARTS ////////////////////////////////
enum
{
	NORMAL_PACKET = 0,
	DIFFERENT_PACKET
};

enum
{
	RX_BUFFER_SIZE   = 272,     //272
	MAIN_BUFFER_SIZE = RX_BUFFER_SIZE +64
};




enum RECV_FLAG
{
	RECEIVE_NOT_COMPLETED = 0,
	RECEIVE_COMPLETED	  = !RECEIVE_NOT_COMPLETED
};




extern enum DATA_FLAGS dataPcktFlag ;
extern enum RECV_FLAG receivedFlag  ;


//////////////////////////////// END OF ENUM PARTS ////////////////////////////////

//////////////////////////////// STRUCT PARTS ////////////////////////////////
struct timerIDS
{
	uint8_t T_ID;
};


struct sUartBuffers // 's' meaning structure
{
	uint8_t RX_Buffer[RX_BUFFER_SIZE];
	uint8_t Main_Uart_Buffer[MAIN_BUFFER_SIZE];
	uint16_t LastReceivedByte;
};


struct dataPackage
{
	uint32_t packageNo		;
	float 	 pressure		;
	uint8_t  packageType	;
};







struct Packet_UART_Interface
{
	UART_HandleTypeDef * const uartInterface ;
	struct dataPackage * const dataPacket    ;
};

// externed Variables



extern enum DATA_FLAGS dataPcktFlag;

extern struct timerIDS TimerArray[2];
extern struct sUartBuffers uartBuffers;
extern struct dataPackage myPacket;
extern struct Packet_UART_Interface PCGK_UART;





//////////////////////////////// END OF STRUCT PARTS ////////////////////////////////



