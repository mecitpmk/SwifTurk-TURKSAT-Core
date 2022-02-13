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

typedef enum
{
	UART_RX_EVENT 	= 0x01,
	TELEMETRI_SEND 	= 0x08

}TSTSTS;


void HAL_UARTEx_RxEventCallback( UART_HandleTypeDef *huart, uint16_t Size )
{

	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	uartBuffers.LastReceivedByte = Size;							      // transfer total received bytes to LastReceivedByte

	uint32_t EVENT = (uint32_t)UART_RX_EVENT ;

	xTaskNotifyFromISR( CommunucationTASK_HANDLE ,
						EVENT ,
						eSetBits ,
						&xHigherPriorityTaskWoken );

	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
	// AMAC = HARDWARE TIMER ICIN HAZIRLIK !

	/* HW Timer Callback ile yapacak isek assagidaki documentasyona gore ;
	 * xTaskToNotify 	: CommunucationTASK_HANDLE
	 * uxIndexToNotify 	: 0 (0.Index yani atıyorum baska bir NotifyBekleyen yer varsa diye bu sekilde.)
	 * ulValue 			: u32 bir value verecegiz buraya atıyorum 0 = TELEMETRI_SEND olsun,
	 * 					  waitINDEXED yapan (xTaskNotifyWaitIndexed) fonksiyon aktive olduğunda
	 * 					  if condition koşacağız if ( TELEMETRI_SEND == STATUS )
	 * 					  VEYA 					 if ( UART_RX_Event  == STATUS )
	 * 					  ona göre bir yonlendirme olacak.
	 * pxHigherPriorityTaskWoken : fonksiyon basinda tanimlayacağız bunu ve adress'ini verecegiz &pxHigherPriorityTaskWoken seklinde.

     * https://www.freertos.org/xTaskNotifyFromISR.html

	 * BaseType_t xTaskNotifyIndexedFromISR( TaskHandle_t xTaskToNotify,
                                       UBaseType_t uxIndexToNotify,
                                       uint32_t ulValue,
                                       eNotifyAction eAction,
                                       BaseType_t *pxHigherPriorityTaskWoken );
	 *
	 */



    uint32_t EVENT = (uint32_t)TELEMETRI_SEND;
    xTaskNotify( CommunucationTASK_HANDLE,
    			EVENT,
				eSetBits);

//    portYIELD();
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

	uint32_t NOTIFIED_VALUE;
	for(;;)
	{

		xTaskNotifyWait( 0, 0xFFFFFFFF  , &NOTIFIED_VALUE, portMAX_DELAY );

		if ( ( TELEMETRI_SEND & NOTIFIED_VALUE ) != 0 )
		{
			if ( ( D_READY == sensorDataRdyFlag ) && ( CONTAINER_DATA_COLLECTED == CONTAINER_TELEM_STATUS ) ) // IF all sensors are readed and container datas have been collected.
			{
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
			}


		}
		if ( ( UART_RX_EVENT & NOTIFIED_VALUE ) != 0 )
		{
			memset( uartBuffers.Main_Uart_Buffer, '\0' , MAIN_BUFFER_SIZE ); // Copy the received bytes to Main Buffers.
			memcpy( uartBuffers.Main_Uart_Buffer, uartBuffers.RX_Buffer , uartBuffers.LastReceivedByte ); // Copy the received bytes to Main Buffers.
			memset( uartBuffers.RX_Buffer , '\0' ,RX_BUFFER_SIZE );

			handleTelemPackage();

			HAL_UARTEx_ReceiveToIdle_DMA( PCGK_UART.uartInterface , uartBuffers.RX_Buffer, RX_BUFFER_SIZE );
			__HAL_DMA_DISABLE_IT( &hdma_usart2_rx , DMA_IT_HT );

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
