/*
 * satellite_task.c
 *
 *  Created on: Jan 9, 2022
 *      Author: Mecit
 */


#include "satellite_tasks.h"
#include "msControl.h"
#include "string.h"
#include "sensors.h"
#include "sdSaver.h"

extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

TaskHandle_t SensorTASK_HANDLE			= NULL;
TaskHandle_t CommunucationTASK_HANDLE	= NULL;
TaskHandle_t MotorControllerTASK_HANDLE	= NULL;

TimerHandle_t sensorTimer		= NULL;
TimerHandle_t healthDataTimer	= NULL;
TimerHandle_t fixedAltTimer		= NULL;



void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef *huart, uint16_t Size )
{

	if ( USART2 == huart->Instance )
	{
		controlFlags.uartRX_ReceiveCmpltd = TRUE;
		uartBuffers.LastReceivedByte = Size;							      // transfer total received bytes to LastReceivedByte
	}
}



void fixAltTimerCLLBCK( TimerHandle_t pxTimer ) // This callback Timer function is also used in 10 sec Motor test.
{
	// end of 10 seconds.
	if ( !controlFlags.MotorTestActive )
	{
		vTaskSuspendAll();
		controlFlags.fixAltitude		= FALSE;
		controlFlags.autoMotorControl 	= TRUE;

		xTaskResumeAll();
	}
	else
	{
		// Deactivate motor in here. (10 sec passed.)
	}

}

void sensorTimerCLLBCK( TimerHandle_t pxTimer )
{

	TickType_t currentTime  = xTaskGetTickCount();
	while ( xTaskGetTickCount() - currentTime < 100 ) // Purpose : if the Below If condition is not satisfied, give the sensor or container 100 ms. Maybe they will satisfied in 100ms tolerance.
	{
		if ( ( D_READY == sensorDataRdyFlag ) && ( CONTAINER_DATA_COLLECTED == CONTAINER_TELEM_STATUS ) ) // IF all sensors are readed and container datas have been collected.
		{
			// send copied 1HZ Telem buffer with  UART in HERE.
			transferContainerDatatoMainStruct(); // Combine Container Datas.
			sendTelemetryData( TELEM_1HZ );
			logData( PAYLOAD_DATA );

			vTaskSuspendAll();
			controlFlags.TELEM_DATA_COPIED  = FALSE	;
			controlFlags.TelemetryNeeded 	= TRUE	;
			sensorDataRdyFlag 				= FALSE	;
			Telemetry_1HZ_Data.PACKET_NUMBER += 1	;
//			CONTAINER_TELEM_STATUS = CONTAINER_DATA_NOT_COLLECTED;  // GCS ile Haberleşme Testi icin Commentledim.
			xTaskResumeAll();
			return;
		}
	}



}
void healthTimerCLLBCK( TimerHandle_t pxTimer )
{
	// Send the strcuture with UART in Here.
	// Deactive for now.


//	sendTelemetryData( TELEM_FLIGHT_HEALTH );
	Telemetry_Health_Data.PACKET_NUMBER += 1;


	return;
}



void SensorTASK( void * pvParameters )
{
	sensorTimer	= xTimerCreate( "sTMR", pdMS_TO_TICKS( ( 1000/SENSOR_DATA_HZ ) ), pdTRUE, NULL, &sensorTimerCLLBCK );
	xTimerStart( sensorTimer , portMAX_DELAY );

	controlFlags.TelemetryNeeded = TRUE;
	for(;;)
	{
		// sensor readings...
		readBMP();
		readIMU();

		// readGPS() => bu fonksiyon içerisinde bir flag olsun, 250 ms de bir (4HZ) yada 2HZ de bir okuyalım .Bunu da HW Timer ile sağlayalım. Flag değişsin, Anlayalım 250 ms geçtiğini ve okuyalım.
		// Telemetrileri HW Timer ile yapmaya bak bir sonraki aşama. (CPU'dan yük kaldıralım hafiften.)

		if ( controlFlags.TelemetryNeeded && !controlFlags.TELEM_DATA_COPIED )
		{
			checkDatasReady();
			if ( D_READY == sensorDataRdyFlag )
			{
				controlFlags.TelemetryNeeded = FALSE;
				// copy the Telemetry to the buffer.
				memcpy ( &Telemetry_1HZ_Data_BUFF , &Telemetry_1HZ_Data , sizeof ( Telemetry_1HZ_Data ) );

				vTaskSuspendAll();
				controlFlags.TELEM_DATA_COPIED = TRUE;
				xTaskResumeAll();
			}

		}
		vTaskDelay( pdMS_TO_TICKS( 200 ) );
	}
	return;
}

void CommunucationTASK( void * pvParameters )
{
	healthDataTimer	= xTimerCreate( "hTMR", pdMS_TO_TICKS( ( 1000/HEALTH_DATA_HZ ) ), pdTRUE, NULL, &healthTimerCLLBCK );
	xTimerStart( healthDataTimer , portMAX_DELAY );
	HAL_UARTEx_ReceiveToIdle_DMA( PCGK_UART.uartInterface , uartBuffers.RX_Buffer, RX_BUFFER_SIZE );
	__HAL_DMA_DISABLE_IT( &hdma_usart2_rx , DMA_IT_HT );
	for(;;)
	{
		// IDLE RX checks is RX flag ready

			// checks async data sending flags. (IMPLEMENT LATER ON)
				// IDLE Rx Event Cllbck makes flag true
					// if ( flags.UartDataAvailable) ... Copy RX Buffer to MainUartBuffer and clean RX Buffer and go to handleTelemPackage();

		// if ( flags.uartDataAvaiable && !controlFlags.isUartHANDLING )
		// memcpy(mainuart , rxbuffer , sizeof mainuart )
		// clearUartBuffer(rxbuffer)
		// controlFlags.isUartHANDLING = TRUE
		// __HAL_ENABLE_RX_IDLE_DMA
		// handleTelemPackage();
		if ( TRUE == controlFlags.uartRX_ReceiveCmpltd )
		{
			cleanUartBuffers( uartBuffers.Main_Uart_Buffer, MAIN_BUFFER_SIZE );   // clear the Main buffer first
			memcpy( uartBuffers.Main_Uart_Buffer, uartBuffers.RX_Buffer , uartBuffers.LastReceivedByte ); // Copy the received bytes to Main Buffers.
			cleanUartBuffers( uartBuffers.RX_Buffer, RX_BUFFER_SIZE );			  // clear the RX buffer after the copying process.

			controlFlags.uartRX_ReceiveCmpltd = FALSE;


			HAL_UARTEx_ReceiveToIdle_DMA( PCGK_UART.uartInterface , uartBuffers.RX_Buffer, RX_BUFFER_SIZE );
			__HAL_DMA_DISABLE_IT( &hdma_usart2_rx , DMA_IT_HT );

			handleTelemPackage();
		}

	}
	return;
}

void MotorControlTASK( void * pvParameters )
{
	fixedAltTimer = xTimerCreate( "fATMR", pdMS_TO_TICKS( ( 10000 ) ), pdFALSE, NULL, &fixAltTimerCLLBCK );
	for(;;)
	{
		if ( SEPEARTING_MECHANISM_ACTIVE == controlFlags.isSeperatingMechanismActive )
		{
			// Seperating
				// sepearteFunction()
			releasePayload();

			vTaskSuspendAll();
			controlFlags.autoMotorControl 				= TRUE;
			controlFlags.isSeperatingMechanismActive 	= FALSE;
			xTaskResumeAll();
		}

		else if ( ( STAT_FIXEDALT == Telemetry_1HZ_Data.FLIGHT_STATUS ) && ( TRUE == controlFlags.fixAltitude ) )
		{
			// Altitude fixB
			static uint8_t flg = 0;
			if (0 == flg) // if the timer is not initalize before.
			{
				flg = 1;
				xTimerStart( fixedAltTimer, portMAX_DELAY ); // start timer.
			}

			if ( TRUE == descentControl.FLAGS.isReady )
			{
				// speed kontrol for Fixing Altitude.
				setRPM();
				descentControl.FLAGS.isReady = FALSE;

			}

		}
		else if ( TRUE == controlFlags.autoMotorControl  )
		{

			if ( TRUE == descentControl.FLAGS.isReady ) // if measurement is not ready pass.
			{
				// speed kontrol. -> this function should call decisionForMotorSpeed
					// do STH.
					setRPM();

				descentControl.FLAGS.isReady = FALSE; // wait for another Measurement.
			}

		}
	}
	return;
}
