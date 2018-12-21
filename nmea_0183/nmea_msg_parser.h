/**
  *@file	nmea_msg_parser.h
  *@brief	NMEA-0183 message process support header file
  *@author	Eggcar
  */
#ifndef __NMEA_MSG_PARSER_H
#define __NMEA_MSG_PARSER_H
#include "nmea_fmt_parser.h"
#include "time_base.h"

#include <stdint.h>

int32_t SetTime_RMC(char *msg, localTime_t *time);

int32_t SetTime_GGA(char *msg, localTime_t *time);

int32_t SetLeap_LSF(char *msg, leapSecond_t *leap);

int32_t ParseLeap_LSF(char *msg, leapSecond_t *leap);

int32_t SetLeap_LPS(char *msg, leapSecond_t *leap);

int32_t ParseLeap_LPS(char *msg, leapSecond_t *leap, localTime_t *time);

int32_t ParseTime_RMC(char *msg, uint32_t len, localTime_t *time);

int32_t ParseSatNum_GSA(char *msg, uint32_t len, int32_t *sysid);

int32_t ParseLocation_GNS(char *msg, uint32_t len, double *latitude, double *longitude, double *altitude);

int32_t ParseCentury_ZDA(char *msg, uint32_t len);

int32_t ParseTime_ZDA(char *msg, uint32_t len, localTime_t *time);

#endif
