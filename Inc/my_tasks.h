/*
 * my_tasks.h
 *
 *  Created on: Dec 30, 2021
 *      Author: Mecit
 */

#pragma once

// Includes
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "main.h"
#include "stdio.h"
#include "enums_structs.h"

// Defines
#define TELEM_TIMER 		0x0
#define MANUAL_MOTOR_TIMER 	0x1
#define UINT8 uint8_t
#define INT8 int8_t



extern TaskHandle_t firstxHandler	             ;
extern TaskHandle_t communucationTaskHandler     ;
extern TaskHandle_t communucationReceiverHandler ;


/* for watermark
static char firstxHandlerArr[50];
static char commTaskHandlerArr[50];
static char communucationReceiverHandlerArr[50];
*/





// Function Prototypes
void timerCLLBCK( TimerHandle_t pxTimer );
void taskFunction( void * pvParameters );
void taskFunctionComm( void * pvParameters );
void taskFunctionCommRecv( void * pvParameters );
