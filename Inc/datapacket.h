/*
 * datapacket.h
 *
 *  Created on: Jan 8, 2022
 *      Author: Mecit
 */

#pragma once
#include "stm32g4xx_hal.h"

#include "enums_structs.h"


typedef enum
{
	TELEM_1HZ 				= 0,
	TELEM_FLIGHT_HEALTH		= 1,
	GCS_TELEM_REQUEST		= 2,
	GCS_TELEM_MISSION		= 3,
	GCS_TELEM_COMMAND		= 4,
	GCS_TELEM_RESPONSE		= 5,
	CONTAINER_TELEMETRY		= 6,
	VIDEO_PACKAGE_REQUEST	= 7,
	UNKNOWN

}TELEMETRY_PACKAGE_TYPES;

typedef enum
{
    STAT_WAITING		= 0,
    STAT_RISING			= 1,
	STAT_FLIGHTFALL		= 2,
	STAT_SEPERATING		= 3,
    STAT_PAYFALL		= 4,
    STAT_FIXEDALT		= 5,
    STAT_RESCUE			= 6
}FLIGHT_STATUS_t;


typedef enum
{
	VIDEO_NOT_SAVED = 0,
	VIDEO_SAVED		= !VIDEO_NOT_SAVED
}VIDEO_TRANSMISSION_STATUS_t;

typedef enum
{
	TELEM_ERROR 	= 0,
	TELEM_VALIDATED = !TELEM_ERROR
}TELEM_VALID_STATUS;


typedef enum
{
	CONTAINER_DATA_NOT_COLLECTED = 0,
	CONTAINER_DATA_COLLECTED 	 = !CONTAINER_DATA_NOT_COLLECTED
}CONTAINER_TELEM_STATUS_t;



typedef enum
{
	MANUAL_RELEASE	= 0,
	TEST_MOTOR		= !MANUAL_RELEASE
}GCS_COMMAND;

struct Telemetry_1HZ
{
	/* Main Telemetry Strcuture , Takes Sensore Data and
	 * Collect Container Telemetry Data.
	 */
	#define TELEM_1HZ_BUFF_SIZE ( 5 ) // If you add sth change the buffer size according to what you add.


	const uint32_t  START_OF_FRAME  	; // <
	const uint8_t  PACKAGE_TYPE	  	; // this is 1Hz telemetry Structure @param => enum PACKAGE_TYPES
	const uint8_t  PACKAGE_SIZE    	;
	const uint16_t TEAM_ID			;
	uint16_t PACKET_NUMBER ;  // will be increased every Hz



	float Temperature  ;
	float Pressure     ;
	float Altitude     ;  // relative to Y-Axis
	float DescentSpeed ;  // relative to Y-Axis
	float GPS_Latitude 		;
	float GPS_Longtitude 	;
	float GPS_Altitude 		;
	float Container_Altitude 		;
	float Container_Pressure		;
	float Container_GPS_Latitude	;
	float Container_GPS_Longtitude	;
	float Container_GPS_Altitude	;
	float AltitudeDifference		;
	float pitch	;
	float roll	;
	float yaw	;

	uint8_t FLIGHT_STATUS ;							// for GCS Define => enum FLIGHT_STATUS_t : byte
	uint8_t VIDEO_TRANSMISSION_STATUS;  //

	const uint32_t END_OF_FRAME ; // >


} __attribute__((packed));



struct Telemetry_Health
{
	#define TELEM_HEALTH_BUFF_SIZE ( 18 ) //If you add sth change the buffer size according to what you add.
	const uint32_t START_OF_FRAME  ; // <

	const uint8_t PACKAGE_TYPE	  ; // this is 1Hz telemetry Structure @param => enum PACKAGE_TYPES
	const uint8_t PACKAGE_SIZE    ;
	uint32_t PACKET_NUMBER 		  ;  	 // will be increased every Hz

	uint16_t Fault_in_Temperature ; // According to our boundries (if outside in our boundries increase counter.)
	uint16_t Fault_in_Pressure    ; // According to our boundries (if outside in our boundries increase counter.)
	uint16_t Fault_in_Altitude	  ; // According to our boundries (if outside in our boundries increase counter.)

	uint32_t Fault_in_GCS_TELEM   ;

	const uint32_t END_OF_FRAME    ; // >

} __attribute__((packed));

typedef struct
{
	const uint32_t START_OF_FRAME  	; // <

	uint8_t PACKAGE_TYPE 		  	;

	uint16_t videoID				;

	uint8_t videoByte[255]			;

	uint8_t isEnd					;
	const uint32_t END_OF_FRAME    	; // >
}__attribute__((packed))GCS_Telemetry_t;

typedef struct
{
	const uint32_t START_OF_FRAME  	; // <

	uint8_t PACKAGE_TYPE 		  	;

	uint8_t COMMAND					;

	const uint32_t END_OF_FRAME    	; // >
}__attribute__((packed))GCS_Command_t;

struct GCS_Tel_Response
{
	#define GCS_TEL_RESPONSE_BUFF_SIZE ( 4 )

	const uint32_t START_OF_FRAME  ; // <

	uint8_t PACKAGE_TYPE 		  ;

	uint8_t RESPONSE_STATUS		  ;

	const uint32_t END_OF_FRAME    ; // >
}__attribute__((packed));

struct Container_Telem
{
	// Container Telemetry Directly Copied to this Structure
	const uint32_t START_OF_FRAME  	; // <

	uint8_t PACKAGE_TYPE			;

	float Container_Altitude 		;
	float Container_Pressure		;
	float Container_GPS_Latitude	;
	float Container_GPS_Longtitude	;
	float Container_GPS_Altitude	;

	const uint32_t END_OF_FRAME		; // >
}__attribute__((packed));

typedef struct
{
	const uint32_t START_OF_FRAME  	; // <

	uint8_t PACKAGE_TYPE			;

	uint16_t videoID				;

	const uint32_t END_OF_FRAME		; // >
}__attribute__((packed))VideoHandler_t;

extern struct Telemetry_1HZ 	Telemetry_1HZ_Data;
extern struct Telemetry_1HZ 	Telemetry_1HZ_Data_BUFF;
extern struct Telemetry_Health 	Telemetry_Health_Data;
extern struct GCS_Tel_Response  GCS_Telemetry_Response;
extern struct Container_Telem   Telemetry_Container;



extern CONTAINER_TELEM_STATUS_t CONTAINER_TELEM_STATUS;
extern GCS_Command_t 			GCS_Command;
extern GCS_Telemetry_t 			GCS_Telemetry;
extern VideoHandler_t			VideoHandler;
void sendTelemetryData( const uint8_t TELEM_PACKAGE_TYPES );
void handleTelemPackage ( void );
void transferContainerDatatoMainStruct( void );
