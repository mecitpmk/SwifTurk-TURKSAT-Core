/*
 * msControl.c
 *
 *  Created on: Jan 9, 2022
 *      Author: Mecit
 */



#include  "msControl.h"
#include  "datapacket.h"
#include  "FreeRTOS.h"
#include  "task.h"


static void calculateDescentSpeed( void );
static void decisionForMotorSpeed( void );

typedef enum
{
	ALT_FIXED		= 0,
	AUTO_CONTROL	= !ALT_FIXED
}FLIGHT_CNTRL;

#define MS_TO_SEC_COEFF 1000

struct MSControl descentControl =  { .LastReadedTime = 0 ,  .FLAGS = {.RESET_FLAGS = 0} , .currentAltitude = 0 , .lastAltitude = 0 , .DescentSpeed = 0};



// Speed Should be bounded in 8-10 m/s
static void decisionForMotorSpeed( void )
{
	descentControl.DescentSpeed = ( ( descentControl.currentAltitude - descentControl.lastAltitude )/ ( (float)( descentControl.currentReadedTime - descentControl.LastReadedTime ) ) )*MS_TO_SEC_COEFF;
	if ( FALSE == controlFlags.TELEM_DATA_COPIED ) // IF the MAIN STRUCTURE NOT COPIED TO BUFFER YET DIRETLY PASTE IT
	{
		vTaskSuspendAll();
		Telemetry_1HZ_Data.DescentSpeed = descentControl.DescentSpeed;
		xTaskResumeAll();
	}
	else // IF THE MAIN STRUCTURE ALLREADY COPIED BUFFER, JUST CHANGE FROM THE BUFFER.
	{
		Telemetry_1HZ_Data_BUFF.DescentSpeed = descentControl.DescentSpeed;
	}



}

static void calculateDescentSpeed( void )
{



	decisionForMotorSpeed(); // Calculates DescentSpeed


	// AUTO ise THRUST'a göre DECREASE-INCREASE-Constant desin., 8-10 m/s'ye göre deği. 8 < x < 10

	int8_t MIN_INTERVAL;
	int8_t MAX_INTERVAL;

	switch ( controlFlags.autoMotorControl )
	{
		case AUTO_CONTROL:
			MIN_INTERVAL = 8;
			MAX_INTERVAL = 10;
			break;
		case ALT_FIXED:
			MIN_INTERVAL = -1;
			MAX_INTERVAL = 1;
			break;
		default:
			break;
	}

	// Decision for speed INCREASE DECREASE OR What?
	if ( descentControl.DescentSpeed <= MIN_INTERVAL )
	{
		descentControl.FLAGS.SPEED_SHOULD = DECREASE;
	}
	else if ( descentControl.DescentSpeed >= MAX_INTERVAL )
	{
		descentControl.FLAGS.SPEED_SHOULD = INCREASE;
	}
	else
	{
		descentControl.FLAGS.SPEED_SHOULD = CONSTANT;
	}

	return ;
}

void setRPM( void )
{


	calculateDescentSpeed();

	switch ( descentControl.FLAGS.SPEED_SHOULD )
	{
		case DECREASE:
			break;
		case INCREASE:
			break;
		case CONSTANT:
			break;
		default :
			break;
	}


//	descentControl.FLAGS.SPEED_ADJUST = SPEED_ADJUSTED;

	return;
}

void releasePayload( void )
{
	// Relaese payload mechanism.
	return ;
}

void testMotorWORK( void )
{
	// This function give a constant speed to the motor.

	return ;
}

