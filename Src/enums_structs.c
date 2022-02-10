/*
 * enums_structs.c
 *
 *  Created on: Dec 30, 2021
 *      Author: Mecit
 */

#include "enums_structs.h"



// extern variables
extern UART_HandleTypeDef huart2;



///////////// INIT VARIABLES ///////////////////
enum DATA_FLAGS dataPcktFlag  = D_NOT_READY;
enum RECV_FLAG receivedFlag   = RECEIVE_NOT_COMPLETED;


// will be deleted.
struct timerIDS TimerArray[2];
struct dataPackage myPacket     		  =  { .packageNo = 0 , .packageType = NORMAL_PACKET , .pressure = 0.0f };


struct sUartBuffers uartBuffers 		  =  { .LastReceivedByte = 0 , .Main_Uart_Buffer = {0} , .RX_Buffer = {0} };
struct Packet_UART_Interface PCGK_UART    =  { .dataPacket = &myPacket , .uartInterface = &huart2 };



///////////// INIT VARIABLES ///////////////////




void cleanUartBuffers(uint8_t * const Buffer , const uint16_t BufferSize)
{
	for ( uint16_t i = 0 ; i < BufferSize ; i++ )
	{
		Buffer[i] = '\0';
	}

}
