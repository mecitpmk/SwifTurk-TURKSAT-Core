/*
 * msControl.h
 *
 *  Created on: Jan 9, 2022
 *      Author: Mecit
 */

#pragma once

//#include  "datapacket.h"

#include "inttypes.h"

#define TRUE 1
#define FALSE 0

// MSCONTROL PARTS
enum
{
	CONSTANT = 0,
	INCREASE,
	DECREASE
};

enum
{
	SPEED_NOT_ADJUSTED = 0,
	SPEED_ADJUSTED     = !SPEED_NOT_ADJUSTED
};

enum
{
	SEPEARTING_MECHANISM_DEACTIVE	= 0,
	SEPEARTING_MECHANISM_ACTIVE		= !SEPEARTING_MECHANISM_DEACTIVE
};

struct MSControl
{

	float    lastAltitude      ;
	float    currentAltitude   ;
	float    DescentSpeed	   ;

	uint32_t LastReadedTime    ;
	uint32_t currentReadedTime ;



	union
	{
		struct
		{
			uint8_t SPEED_ADJUST   : 1 ;
			uint8_t SPEED_SHOULD   : 2 ; // CONSTANT, DECREASE , INCREASE
			uint8_t isReady		   : 1 ;
		};
		uint8_t RESET_FLAGS;
	}FLAGS;

};

extern struct MSControl descentControl;


void setRPM( void );
void releasePayload( void );
void testMotorWORK( void );

