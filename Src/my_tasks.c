/*
 * my_tasks.c
 *
 *  Created on: Dec 30, 2021
 *      Author: Mecit
 */


#include "my_tasks.h"







extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;



TaskHandle_t firstxHandler	              = NULL;
TaskHandle_t communucationTaskHandler     = NULL;
TaskHandle_t communucationReceiverHandler = NULL;

//static char firstxHandlerArr[50];
//static char commTaskHandlerArr[50];
//static char communucationReceiverHandlerArr[50];


//void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef *huart, uint16_t Size )
//{
//
//	if ( USART2 == huart->Instance )
//	{
//
//		//		vTaskSuspendAll(); // NOT SAFE CHECK WHAT CAN WE DO
//		receivedFlag = RECEIVE_COMPLETED; // to prevent race conditions.
//		uartBuffers.LastReceivedByte = Size;							      // transfer total received bytes to LastReceivedByte
////		xTaskResumeAll();  // NOT SAFE CHECK WHAT CAN WE DO
//
//
//
////
//
//	}
//}



void timerCLLBCK( TimerHandle_t pxTimer )
{

	struct timerIDS *specificTimer = ( struct timerIDS * )pvTimerGetTimerID(pxTimer);

	static const char *TELEM_MES = "TELEM TIMER ACTIVATED.\r\n";
	static const char *MOTOR_MES = "MANUAL MOTOR DEACTIVATING..\r\n";
	static const char *ERROR_MES = "DATA COULDNT READ SPECIFIC INTERVAL!\r\n";
	static const char *DT_OK_MES = "DATA IS READY SENDING..\r\n";
	static const char *WAITI_MES = "WAITING FOR DATA...\r\n";
	static char telemetryINFO[60] ;

	switch (specificTimer->T_ID)
	{
		case TELEM_TIMER:

//				HAL_UART_Transmit( &huart2, ( UINT8* )TELEM_MES, strlen( TELEM_MES ), 500 );
				while ( HAL_UART_Transmit_DMA( &huart2, ( uint8_t * )TELEM_MES , strlen( TELEM_MES ) ) != HAL_OK);
//				sendWith_UART_DMA( &huart2, ( uint8_t * )TELEM_MES , strlen( TELEM_MES ) );

				// if the telemetry is not ready wait 150 ms until  its ready.
				if (D_READY != dataPcktFlag)
				{
					UINT8 printFlag = 0;
					TickType_t countDownTimer = xTaskGetTickCount();
					while (xTaskGetTickCount() - countDownTimer < 150)
					{
						if (0 == printFlag)
						{
							printFlag = 1;
//							HAL_UART_Transmit( &huart2, ( UINT8* )WAITI_MES , strlen(WAITI_MES), 500 ); // Print waiting for data only just one time.
							while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )WAITI_MES , strlen(WAITI_MES) ) != HAL_OK ); // Print waiting for data only just one time.
						}
						if (D_READY == dataPcktFlag) break; // If the data ready break and continue.
					}
					if (D_NOT_READY == dataPcktFlag) // After while loop dataPcktFlag may didn't change so that check it one more time.
					{
						// PLEASE ON ERROR FLAG IN HERE.
//						HAL_UART_Transmit( &huart2, ( UINT8* )ERROR_MES , strlen(ERROR_MES), 500 ); // Error Message (DATA COULDNT READ SPECIFIC INTERVAL!)
						HAL_UART_Transmit_DMA( &huart2, ( UINT8* )ERROR_MES , strlen(ERROR_MES) ); // Error Message (DATA COULDNT READ SPECIFIC INTERVAL!)
						return;
					}
				}

				// if The Telemetry datas ready go further.
//				HAL_UART_Transmit( &huart2, ( UINT8* )DT_OK_MES , strlen(DT_OK_MES), 500 );
				while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )DT_OK_MES , strlen(DT_OK_MES)  ) != HAL_OK);

				vTaskSuspendAll(); // Prevent race condition.
				dataPcktFlag = D_NOT_READY;
				xTaskResumeAll();  // Prevent race condition.

				sprintf( telemetryINFO,"Package No : %ld , Pressure : %.2f \r\n",myPacket.packageNo,myPacket.pressure );
//				HAL_UART_Transmit( &huart2, ( UINT8* )telemetryINFO, strlen( telemetryINFO ), 500 );
				while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )telemetryINFO, strlen( telemetryINFO ) )  != HAL_OK );

				break;
		case MANUAL_MOTOR_TIMER:
				//Deactivate Manual Motor in Here.
//				HAL_UART_Transmit( &huart2, ( UINT8* )MOTOR_MES, strlen( MOTOR_MES ), 500 ); // Printing a message for does it work or not?
				while (HAL_UART_Transmit_DMA( &huart2, ( UINT8* )MOTOR_MES, strlen( MOTOR_MES ) ) != HAL_OK); // Printing a message for does it work or not?
				break;

		default:
			break;
	}

}


void taskFunctionCommRecv( void * pvParameters )
{
	static const char *NotifyMesTake = "Notify Taked From taskFunction\r\n";
	static const char *NotifyMesGive = "Now Notify Giving to taskFunction\r\n";
	static char  LEDMesBuffer[30];
	static TickType_t currentTime; // Save TickCount in here.


	// WaterMark
//	static char AvBytes[30];
//	UBaseType_t uxHighWaterMark;
//    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

	for ( ;; )
	{
		ulTaskNotifyTake( pdTRUE , portMAX_DELAY );       // wait UNTIL taskFunction gives Permission.

//		HAL_UART_Transmit( &huart2, ( UINT8* )NotifyMesTake, strlen( NotifyMesTake ), 500 ); //Print Notify Taked From taskFunction
		while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )NotifyMesTake, strlen( NotifyMesTake ) ) != HAL_OK); //Print Notify Taked From taskFunction
		UINT8 ledCounter = 1;

		currentTime = xTaskGetTickCount(); // save current tickcount.
		while ( ( xTaskGetTickCount() - currentTime ) < 500 ) // Until 500 TC Led On//off and Print a Message
		{
			HAL_GPIO_TogglePin( LD2_GPIO_Port, LD2_Pin );
			sprintf(LEDMesBuffer,"LED_MESSAGE[-> %u ]\r\n",ledCounter);
			ledCounter += 1;

//			HAL_UART_Transmit( &huart2, ( UINT8* )LEDMesBuffer, strlen( LEDMesBuffer ), 500 );
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )LEDMesBuffer, strlen( LEDMesBuffer ) ) != HAL_OK);

			vTaskDelay(pdMS_TO_TICKS( 80 )); // Wait 80ms.
		}
//		HAL_UART_Transmit( &huart2, ( UINT8* )NotifyMesGive, strlen( NotifyMesGive ), 500 );
		while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )NotifyMesGive, strlen( NotifyMesGive ) ) != HAL_OK);

		/* WaterMark
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		sprintf(AvBytes , "tskfnct -> Av:  %ld \n\r",uxHighWaterMark);
		while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )AvBytes, strlen( AvBytes ) ) != HAL_OK);
		*/

		xTaskNotifyGive( firstxHandler );  // // now Locked this funct down and Give Permission to taskFunction.

	}
}

/*This function a model of sensor readings and activates Descent Control function with taskNotify
 *
 */
void taskFunction( void * pvParameters )
{
	static const char *ButtonMessage  	  = "Button Has been Pressed !\r\n";
	static const char *NotifyMesGive	  = "Now Notify Giving To CommRecv\r\n";
	static const char *NotifyMesTake	  = "Notify Taked from CommRecv\r\n";
	static UINT8 BUTTON_STATE      		  = 0;
	TimerArray[MANUAL_MOTOR_TIMER].T_ID   = MANUAL_MOTOR_TIMER; // timerCallback will understand what should it do according to timer IDS
	TimerHandle_t manualMotorTimer = xTimerCreate( "manualMotTimer", pdMS_TO_TICKS( 2000 ), pdFALSE, ( void * )&TimerArray[MANUAL_MOTOR_TIMER], timerCLLBCK );


	//WaterMark
//	static char AvBytes[30];
//	UBaseType_t uxHighWaterMark;
//    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );

	for( ;; )
	{
		if ( ( HAL_GPIO_ReadPin( blueButton_GPIO_Port, blueButton_Pin ) == GPIO_PIN_SET ) && ( BUTTON_STATE != 1 ) ) /* This state could be I2C readed sensor */
		{
			BUTTON_STATE = 1;
			//Activate Motor in Here for 10 seconds (After 10 seconds timer will Rise and make it Off!)
			xTimerStart( manualMotorTimer, portMAX_DELAY); // Changed from xTimerStart( manualMotorTimer, 0)
			HAL_GPIO_TogglePin( LD2_GPIO_Port, LD2_Pin );

//			HAL_UART_Transmit( &huart2, ( UINT8* )ButtonMessage, strlen( ButtonMessage ), 500 ); //"Button Has been Pressed !"
//			HAL_UART_Transmit( &huart2, ( UINT8* )NotifyMesGive, strlen( NotifyMesGive ), 500 ); //"Now Notify Giving To CommRecv"
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )ButtonMessage, strlen( ButtonMessage ) )  != HAL_OK); //"Button Has been Pressed !"
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )NotifyMesGive, strlen( NotifyMesGive ) ) != HAL_OK); //"Now Notify Giving To CommRecv"

			xTaskNotifyGive( communucationReceiverHandler ); // Wake up To taskFunctionCommRecv function and wait till the Take Notify from taskFunctionCommRecv!

			ulTaskNotifyTake( pdTRUE , portMAX_DELAY );       // Take it Back!.
//			HAL_UART_Transmit( &huart2, ( UINT8* )NotifyMesTake, strlen( NotifyMesTake ), 500 );
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )NotifyMesTake, strlen( NotifyMesTake ) ) != HAL_OK);

			vTaskDelay( pdMS_TO_TICKS( 50 ) );
		}

		/* WaterMark
		uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
		sprintf(AvBytes , "taskFnction -> Av:  %ld \n\r",uxHighWaterMark);
		while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8* )AvBytes, strlen( AvBytes ) ) != HAL_OK);
		vTaskDelay(pdMS_TO_TICKS(200));
		*/

		BUTTON_STATE = 0;
	}
}



// This function an example of triggering Timer and prevents race conditions with vTaskSuspendAll
void taskFunctionComm( void * pvParameters )
{


	static char MessageBuffer[50];

	static uint32_t COUNTER    = 0;
	static uint8_t LOCK_TIMER  = 0;
	static const char * const PREPARE_MES 	  = "DATA HAS BEEN PREPARED!\r\n";
	static const char * const RX_MES_RECEIVED = "Rx Message Has been Received\r\n";

	TimerArray[TELEM_TIMER].T_ID   = TELEM_TIMER;
	TimerHandle_t telemetryTimerHandle = xTimerCreate( "timerTest", pdMS_TO_TICKS( 1000 ), pdTRUE, ( void * )&TimerArray[TELEM_TIMER], timerCLLBCK );

	// WaterMark
//	UBaseType_t uxHighWaterMark;
//  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
//	static char AvailableBuf[30];




	  /* Activate Async UART DMA  .*/
	HAL_UARTEx_ReceiveToIdle_DMA( &huart2 , uartBuffers.RX_Buffer, RX_BUFFER_SIZE );
	__HAL_DMA_DISABLE_IT( &hdma_usart2_rx , DMA_IT_HT );
	/* Activate Async UART DMA  .*/

	static TickType_t MS_CT ;
	static uint8_t timerActv = 0;
	for( ;; )
	 {
		if (timerActv == 0)
		{
			MS_CT = xTaskGetTickCount();
			timerActv = 1;
		}
		sprintf(MessageBuffer,"%ld - This is From Rtos!\r\n",COUNTER);
		if (xTaskGetTickCount() - MS_CT > 500)
		{
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )MessageBuffer, strlen( MessageBuffer ) ) != HAL_OK); //  [Counter ] - This is From Rtos!
			timerActv = 0;
		}


		if ( RECEIVE_COMPLETED == receivedFlag ) // if the data transmission has been identified and processed
		{

			cleanUartBuffers( uartBuffers.Main_Uart_Buffer, MAIN_BUFFER_SIZE );   // clear the Main buffer first
			memcpy( uartBuffers.Main_Uart_Buffer, uartBuffers.RX_Buffer , uartBuffers.LastReceivedByte ); // Copy the received bytes to Main Buffers.
			cleanUartBuffers( uartBuffers.RX_Buffer, RX_BUFFER_SIZE );			  // clear the RX buffer after the copying process.

			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )RX_MES_RECEIVED, strlen( RX_MES_RECEIVED ) ) != HAL_OK ); // print RX Message has been received
			while ( HAL_UART_Transmit_DMA( &huart2, uartBuffers.Main_Uart_Buffer, uartBuffers.LastReceivedByte )  != HAL_OK ); // print Received Message

			receivedFlag = RECEIVE_NOT_COMPLETED;
			HAL_UARTEx_ReceiveToIdle_DMA( &huart2 , uartBuffers.RX_Buffer, RX_BUFFER_SIZE ); // Activate UART DMA Again. Why? Because after the event Occures, DMA will be stopped Thats why.
			__HAL_DMA_DISABLE_IT( &hdma_usart2_rx , DMA_IT_HT );						     // Disable HALF Interrupt

		}


		if ( ( 0 == COUNTER % 5 ) && ( 0 == LOCK_TIMER ) )
		{
			xTimerStart( telemetryTimerHandle, portMAX_DELAY);
			LOCK_TIMER = 1;
		}

		if (xTaskGetTickCount() - MS_CT > 500)
		{
			vTaskSuspendAll();
	//		HAL_UART_Transmit( &huart2, ( UINT8 * )PREPARE_MES, strlen( PREPARE_MES ), 500 );
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )PREPARE_MES, strlen( PREPARE_MES ) ) != HAL_OK);


			// WaterMark
//			uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
//			sprintf(AvailableBuf , "TaskFnct Comm -> Av : %ld\n\r",uxHighWaterMark);
//			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )AvailableBuf, strlen( AvailableBuf) ) != HAL_OK);


			/*
			uxHighWaterMark = uxTaskGetStackHighWaterMark( firstxHandler );
			sprintf( firstxHandlerArr , "frstx Av : %ld \n\r",uxHighWaterMark);
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )firstxHandlerArr, strlen( firstxHandlerArr) ) != HAL_OK);

			uxHighWaterMark = uxTaskGetStackHighWaterMark( communucationTaskHandler );
			sprintf( commTaskHandlerArr , "CM HNDLR-> Av : %ld \n\r",uxHighWaterMark);
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )commTaskHandlerArr, strlen( commTaskHandlerArr) ) != HAL_OK);

			uxHighWaterMark = uxTaskGetStackHighWaterMark( communucationReceiverHandler );
			sprintf( communucationReceiverHandlerArr, "RCVR Comm -> Av : %ld \n\r",uxHighWaterMark);
			while ( HAL_UART_Transmit_DMA( &huart2, ( UINT8 * )communucationReceiverHandlerArr, strlen( communucationReceiverHandlerArr) ) != HAL_OK);
			*/

			myPacket.packageNo  += 1;
			myPacket.packageType = NORMAL_PACKET;
			myPacket.pressure   += 2.1f;
			dataPcktFlag		 = D_READY;
			xTaskResumeAll();
			timerActv = 0;
		}


		COUNTER++;
		vTaskDelay(pdMS_TO_TICKS(50));
	 }
}


