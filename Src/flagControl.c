/*
 * flagControl.c
 *
 *  Created on: Jan 9, 2022
 *      Author: Mecit
 */

#include "flagControl.h"
#include "FreeRTOS.h"
#include "task.h"



enum DATA_FLAGS sensorDataRdyFlag = D_NOT_READY;
union controlPanel controlFlags   =  { .RESET_FLAG = 0};




void checkDatasReady( void )
{


	if ( ( D_READY == controlFlags.ALTI_RDY ) && ( D_READY == controlFlags.IMU_READY ) && ( D_READY == controlFlags.PRES_RDY ) &&
		 ( D_READY == controlFlags.TEMP_RDY ) && ( D_READY == controlFlags.GPS_READY) )
	{
		controlFlags.ALTI_RDY 	= D_NOT_READY;
		controlFlags.PRES_RDY 	= D_NOT_READY;
		controlFlags.TEMP_RDY 	= D_NOT_READY;
		controlFlags.IMU_READY	= D_NOT_READY;
		controlFlags.GPS_READY	= D_NOT_READY;
		vTaskSuspendAll();
		sensorDataRdyFlag		= D_READY;
		xTaskResumeAll();
	}
	else
	{
		sensorDataRdyFlag = D_NOT_READY;
	}

}

