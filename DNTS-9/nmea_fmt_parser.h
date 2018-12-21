/**
  *@file	nmea_fmt_parser.h
  *@brief	NMEA-0183 message process support header file
  *@author	Eggcar
  */

#ifndef __NMEA_FMT_PARSER_H
#define __NMEA_FMT_PARSER_H

#define MAX_MESSAGE_LENGTH_NMEA		((uint32_t)256)
#define MAX_TYPE_LENGTH_NMEA		((uint32_t)32)

#include <stdint.h>
/**
  *@enum msgType_NMEA_t
  *@brief Enumeration of unicore NMEA 0183 extension message types.
  *
  */
typedef enum {
	GGA,		
	GLL,
	GSA,
	GSV,
	RMC,
	VTG,
	ZDA,
	GST,
	GNS,
	NAVPOS,
	NAVVEL,
	NAVTIME,
	NAVACC,
	ANTSTAT,
	TIMTP,
	TIMEM,
	LSF,
	LPS,
	SIGNAL,
	IPADD,
	UNDEFINED
} msgType_NMEA_t;

/**
  *@enum sysIdentifier_NMEA_t
  *@brief Enumeration of the satellite system identifier.
  */
typedef enum {
	GP,
	BD,
	GB,
	GL,
	GA,
	GN,
	NONE
} sysIdentifier_NMEA_t;

int8_t IfLegal_NMEA(const char * msg, uint32_t len);

msgType_NMEA_t GetMsgType_NMEA(char * msg, uint32_t len);

sysIdentifier_NMEA_t GetSysId_NMEA(char * msg, uint32_t len);

int8_t ContainMsgSum_NMEA(const char * msg, uint32_t len);

int8_t CalcMsgSum_NMEA(const char * msg, uint32_t len, uint16_t * chksum);

int8_t SetMsgSum_NMEA(char * msg, uint32_t len, uint16_t checkSum);

int8_t CheckMsgSum_NMEA(const char * msg, uint32_t len);

int16_t CountMsgElement_NMEA(const char * msg, uint32_t len);

int16_t GetMsgElement_NMEA(char * msg, uint32_t len, uint16_t index, char ** elem);

int8_t SetMsgElement_NMEA(char * msg, uint32_t len, uint16_t index, const char * element);

int16_t LocateMsgElement_NMEA(char * msg, uint32_t len, uint16_t index);

int16_t InsertMsgElement_NMEA(char * msg, uint32_t len, uint16_t idnex, char * element);

#endif
