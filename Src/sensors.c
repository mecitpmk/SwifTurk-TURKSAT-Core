/*
 * sensor.c
 *
 *  Created on: Jan 9, 2022
 *      Author: Mecit
 */


#include "sensors.h"
#include "msControl.h"
#include "datapacket.h"

#include "FreeRTOS.h"
#include "task.h"

#define TRUE 1
#define FALSE 0

struct Sensors_Boundry Sensor_Boundries	= { .MIN_ALT = 0 ,  .MAX_ALT = 1000 , .MIN_PRESS = 0 , .MAX_PRESS = 18453 , .MIN_TEMP = 0 , .MAX_TEMP = 40 ,
										    .MIN_PITCH = 0 ,.MAX_PITCH = 34,  .MIN_ROLL = 0 , .MAX_ROLL = 34 , .MIN_YAW = 0 , .MAX_YAW = 34};


struct IMU IMU_Data			= {0};
struct BMP_Sensor BMP_Data	= {0};


static uint8_t MANIPULATING_FALLING = 0;

void readIMU( void )
{
	// API
	IMU_Data.PITCH 	= 32;
	IMU_Data.ROLL 	= 32;
	IMU_Data.YAW 	= 32;
	static BOUNDRY_RESULT boundryResult;
	boundryResult = checkIMUBoundries();
	if (INSIDE_BOUNDRY == boundryResult)
	{
		vTaskSuspendAll();
		Telemetry_1HZ_Data.pitch = IMU_Data.PITCH;
		Telemetry_1HZ_Data.roll  = IMU_Data.ROLL;
		Telemetry_1HZ_Data.yaw   = IMU_Data.YAW;
		controlFlags.IMU_READY = 1;
		controlFlags.GPS_READY = 1; // for NOW
		xTaskResumeAll();
	}
	else
	{
		// Don't touch the controlFlag for the IMU.
	}

}

void readBMP( void )
{
	// API

	if ( MANIPULATING_FALLING  && !controlFlags.fixAltitude )
	{
		BMP_Data.PRESSURE -= 5;
		BMP_Data.TEMPERATURE = 32;
		if ( !( BMP_Data.ALTITUDE <= 20 ) )
		{
			BMP_Data.ALTITUDE -= 15;
		}

	}
	else if ( !MANIPULATING_FALLING && !controlFlags.fixAltitude && !controlFlags.autoMotorControl )
	{
		if ( Telemetry_1HZ_Data.Altitude > 792 && Telemetry_1HZ_Data.Altitude < 805)
		{
			MANIPULATING_FALLING = TRUE;
		}
		BMP_Data.PRESSURE += 5;
		BMP_Data.TEMPERATURE = 32;
		BMP_Data.ALTITUDE += 10;
	}

	TickType_t readingTime;
	readingTime = xTaskGetTickCount();

	BOUNDRY_RESULT boundryResult ;
	boundryResult = checkBMPBoundries();

	if (INSIDE_BOUNDRY == boundryResult)
	{
		vTaskSuspendAll();
		controlFlags.ALTI_RDY = 1;
		controlFlags.PRES_RDY = 1;
		controlFlags.TEMP_RDY = 1;
		Telemetry_1HZ_Data.Altitude		= BMP_Data.ALTITUDE	     ;
		Telemetry_1HZ_Data.Pressure		= BMP_Data.PRESSURE      ;
		Telemetry_1HZ_Data.Temperature	= BMP_Data.TEMPERATURE   ;

		descentControl.LastReadedTime    = descentControl.currentReadedTime;
		descentControl.currentReadedTime = readingTime;
		descentControl.lastAltitude 	 = descentControl.currentAltitude;
		descentControl.currentAltitude   = BMP_Data.ALTITUDE;
		descentControl.FLAGS.isReady     = TRUE; // According to this flag auto motor contorl will be satisfied.
		xTaskResumeAll();

		findFlightStatus();
	}
	else
	{
		// Don't touch the controlFlag for the bmp.
	}

}



BOUNDRY_RESULT checkIMUBoundries( void )
{

//	if ( (( Sensor_Boundries.MIN_PITCH <Telemetry_1HZ_Data.pitch ) && (Sensor_Boundries.MAX_PITCH > Telemetry_1HZ_Data.pitch )) != 1 )
//	{
//		// Outside boundries.
//	}
	return INSIDE_BOUNDRY;
}

BOUNDRY_RESULT checkBMPBoundries( void )
{
	BOUNDRY_RESULT rslt ;
	rslt = INSIDE_BOUNDRY;

//	if ( (( Sensor_Boundries.MIN_PRESS <Telemetry_1HZ_Data.Pressure ) && (Sensor_Boundries.MAX_PRESS > Telemetry_1HZ_Data.Pressure )) != 1 )
//	{
//		rslt = OUTSIDE_BOUNDRY;
//		vTaskSuspendAll();
//		Telemetry_Health_Data.Fault_in_Pressure += 1;
//		xTaskResumeAll();
//	}
//	if ( (( Sensor_Boundries.MIN_TEMP <Telemetry_1HZ_Data.Temperature ) && (Sensor_Boundries.MAX_TEMP > Telemetry_1HZ_Data.Temperature )) != 1 )
//	{
//		rslt = OUTSIDE_BOUNDRY;
//		vTaskSuspendAll();
//		Telemetry_Health_Data.Fault_in_Temperature += 1;
//		xTaskResumeAll();
//	}
//	if ( (( Sensor_Boundries.MIN_ALT <Telemetry_1HZ_Data.Altitude ) && (Sensor_Boundries.MAX_ALT > Telemetry_1HZ_Data.Altitude )) != 1 )
//	{
//		rslt = OUTSIDE_BOUNDRY;
//		vTaskSuspendAll();
//		Telemetry_Health_Data.Fault_in_Altitude += 1;
//		xTaskResumeAll();
//	}
	return rslt;

}

void findFlightStatus( void )
{
	vTaskSuspendAll();
    if ( Telemetry_1HZ_Data.PACKET_NUMBER == 1 || (Telemetry_1HZ_Data.Altitude >= 0 &&  Telemetry_1HZ_Data.Altitude < 4   && !controlFlags.seperatedBefore ) )
    {
        // ilk açıldığında ve yükseklik   =   0 <= yükseklik < 3
        Telemetry_1HZ_Data.FLIGHT_STATUS	= STAT_WAITING;
        descentControl.DescentSpeed = 0;
    }

    else if (Telemetry_1HZ_Data.PACKET_NUMBER  != 1 && Telemetry_1HZ_Data.Altitude > descentControl.lastAltitude && Telemetry_1HZ_Data.Altitude  >= 4)
    {
        // Şimdikik yükseklik  > önceki yükseklik ve  yükselik >= 4
    	Telemetry_1HZ_Data.FLIGHT_STATUS	= STAT_RISING;
    }

    else if (Telemetry_1HZ_Data.PACKET_NUMBER  != 1 &&  405 >= Telemetry_1HZ_Data.Altitude && 395 <= Telemetry_1HZ_Data.Altitude && !controlFlags.seperatedBefore)
    {
        // 395<yükseklik<405 ve dahaönceAyrışmadıYSA
        // Ayrışma Mekanizmasını Devreye Sok.
    	Telemetry_1HZ_Data.FLIGHT_STATUS			= STAT_SEPERATING    ;
        controlFlags.seperatedBefore				= TRUE               ;
        controlFlags.isSeperatingMechanismActive	= SEPEARTING_MECHANISM_ACTIVE;

        // Seperating Mechanism should change the flag
//        controlFlags.autoMotorControl = TRUE;

    }

    else if (Telemetry_1HZ_Data.PACKET_NUMBER  != 1 && Telemetry_1HZ_Data.Altitude < descentControl.lastAltitude && controlFlags.seperatedBefore && Telemetry_1HZ_Data.Altitude >= 190 && Telemetry_1HZ_Data.Altitude <= 210 && !controlFlags.fixAltitudeBefore)
    {
        //      190 < x < 210 // 185
        // Serial.println("NOW ALTITUDE IS FIXING!!!!!!!!*********");
        controlFlags.fixAltitude			= TRUE           ;
        controlFlags.fixAltitudeBefore		= TRUE           ;
        Telemetry_1HZ_Data.FLIGHT_STATUS	= STAT_FIXEDALT  ;


//        timerPackage.timerfixedAlt =  xTimerCreate("FAlt", pdMS_TO_TICKS(10000), pdFALSE, ( void * )( (uint8_t)timerID_FIXED_ALT ), &Communucation::timerHandlerFunct);
//        xTimerStart(timerPackage.timerfixedAlt,0); // enable.

        /*

        */
        // Burada hemen motora güç ver çünkü diğer yerlere gidene kadar time elapsed olacak.
    }

    else if ( Telemetry_1HZ_Data.PACKET_NUMBER  != 1 && ( TRUE == controlFlags.seperatedBefore ) && ( Telemetry_1HZ_Data.Altitude >= 0 ) && ( Telemetry_1HZ_Data.Altitude <= 20 ) )
    {
        // AYrıştıysa ve  0<= Yükseklik < 5
        // strcpy(data.FLIGHT_STATUS,"RESCUE");
    	Telemetry_1HZ_Data.FLIGHT_STATUS  =  STAT_RESCUE;
    }

    else if (Telemetry_1HZ_Data.PACKET_NUMBER  != 1 && Telemetry_1HZ_Data.Altitude < descentControl.lastAltitude  && controlFlags.seperatedBefore)
    {
    	Telemetry_1HZ_Data.FLIGHT_STATUS	= STAT_PAYFALL;
    	controlFlags.autoMotorControl		= TRUE;
    }



    else if (Telemetry_1HZ_Data.PACKET_NUMBER != 1 && Telemetry_1HZ_Data.Altitude < descentControl.lastAltitude )
    {
        // Şimdiki Yükseklik < Önceki Yükseklik.
        // strcpy(data.FLIGHT_STATUS,"FLIGHTFALL");
    	Telemetry_1HZ_Data.FLIGHT_STATUS	=  STAT_FLIGHTFALL;
    }

	xTaskResumeAll();
}
