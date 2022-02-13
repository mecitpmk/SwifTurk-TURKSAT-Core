/*
 * flagControl.h
 *
 *  Created on: Jan 9, 2022
 *      Author: Mecit
 */

#pragma once

#include "datapacket.h"



enum DATA_FLAGS
{
	D_NOT_READY	= 0,
	D_READY		= !D_NOT_READY
};


union controlPanel
{
	struct
	{
		uint32_t IMU_READY	: 1;
		uint32_t GPS_READY	: 1;
		uint32_t TEMP_RDY	: 1;
		uint32_t PRES_RDY	: 1;
		uint32_t ALTI_RDY	: 1;


        uint32_t fixAltitude			: 1 ;
        uint32_t fixAltitudeBefore		: 1 ;
        uint32_t seperatedBefore		: 1 ;
        uint32_t autoMotorControl		: 1 ;
        uint32_t TelemetryNeeded		: 1 ;
        uint32_t manualReleaseActive	: 1 ;
        uint32_t MotorTestActive		: 1 ;
        uint32_t readyToSendSavedVideo	: 1 ;
        uint32_t isUartHANDLING			: 1	;
        uint32_t isSeperatingMechanismActive : 1;

        uint32_t TELEM_DATA_COPIED				: 1 ;
        uint32_t isvideoSendingProcessActive 	: 1	;

	};



	uint32_t RESET_FLAG;

};

extern enum DATA_FLAGS sensorDataRdyFlag;
extern union controlPanel controlFlags;


void checkDatasReady( void );
