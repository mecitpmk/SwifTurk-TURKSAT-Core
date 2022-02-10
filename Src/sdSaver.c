/*
 * sdSaver.c
 *
 *  Created on: Feb 8, 2022
 *      Author: Mecit
 */

#include "sdSaver.h"
#include "datapacket.h"

void logData ( const uint8_t SavingType )
{
	switch ( SavingType )
	{
		case VIDEO_DATA:
			//Access to SD CARD.
			break;
		case PAYLOAD_DATA:
		{
//			static char SD_Buffer[150];
//			static const char * const SATELLEITE_STATE_STR[] 	= {"Bekleme","Yukselme","Model Uydu Inis","Ayrilma","Gorev Yuku Inis","Irtifa Sabit","Kurtarma"};
//			static const char * const VIDEO_STATE_STR[]			= {"Hayir","Evet"};

			//			sprintf( SD_Buffer, "%d,%d,%d/%d/%d,%d/%d/%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%f,%f,%f,%f,%f,%f,%s,%.2f,%.2f,%.2f,%d,%s",
//					Telemetry_1HZ_Data.TEAM_ID,Telemetry_1HZ_Data.PACKET_NUMBER , Telemetry_1HZ_Data,2,8,2021,12,25,0,0,
//					Telemetry_1HZ_Data.Pressure,Telemetry_1HZ_Data.Container_Pressure,Telemetry_1HZ_Data.Altitude,Telemetry_1HZ_Data.Container_Altitude,);
			break;
		}

		default:
			break;
	}
}
