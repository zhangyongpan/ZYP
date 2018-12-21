/**
  *@file	nmea_msg_parser.c
  *@brief	NMEA-0183 message process support
  *@author	Eggcar
  */

#include "nmea_msg_parser.h"

#include "nmea_fmt_parser.h"
#include "time_base.h"

#include <ctype.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char lastLPS[MAX_MESSAGE_LENGTH_NMEA];
static int32_t currCentury = 2000;

/**
  *@brief	Set UTC time info in a formated RMC message.
  *@param	*msg	array of the formated RMC message
  *@param	*time	time structure contains UTC time
  *@retval	0	Successfully set the RMC message
  *			-1	*msg is not a valid RMC message
  *			-2	*msg length larger than MAX_MESSAGE_LENGTH_NMEA
  */
int32_t SetTime_RMC(char *msg, localTime_t *time)
{
	char tod[11] = {'\0'};  // time of day
	char date[7] = {'\0'};  // date
	uint32_t len;
	int32_t year;
	int32_t err;

	uint16_t checkSum;

	len = strlen(msg);
	err = IfLegal_NMEA(msg, len);
	if (err != 0) {
		return err;
	}
	else if (RMC != GetMsgType_NMEA(msg, len)) {
		return -1;
	}
	else {
		year = time->utc_Year % 100;
		sprintf(tod, "%02" PRIi16 "%02" PRIi16 "%02" PRIi16 ".%03" PRIi16 "", time->utc_Hour, time->utc_Minute, time->utc_Second, (time->utc_Nanosecond / 1000000));
		sprintf(date, "%02" PRIi16 "%02" PRIi16 "%02" PRIi32, time->utc_Day, time->utc_Month, year);
		SetMsgElement_NMEA(msg, len, 1, tod);
		SetMsgElement_NMEA(msg, len, 9, date);
		err = CalcMsgSum_NMEA(msg, len, &checkSum);
		SetMsgSum_NMEA(msg, len, checkSum);
		return 0;
	}
}

/**
 *@brief	Set UTC time info in a formated GGA message.
 *@param	*msg	array of the formated GGA message
 *@param	*time	time structure contains UTC time
 *@retval	0	Successfully set the GGA message
 *			-1	*msg is not a valid GGA message
 *			-2	*msg length larger than MAX_MESSAGE_LENGTH_NMEA
 */
int32_t SetTime_GGA(char *msg, localTime_t *time)
{
	char tod[11] = {'\0'};
	int32_t err;
	uint32_t len;
	uint16_t checkSum;

	len = strlen(msg);
	err = IfLegal_NMEA(msg, len);
	if (err != 0) {
		return err;
	}
	else if (GGA != GetMsgType_NMEA(msg, len)) {
		return -1;
	}
	else {
		sprintf(tod, "%02" PRIi16 "%02" PRIi16 "%02" PRIi16 ".%03" PRIi16 "", time->utc_Hour, time->utc_Minute, time->utc_Second, (time->utc_Nanosecond / 1000000));
		SetMsgElement_NMEA(msg, len, 1, tod);
		err = CalcMsgSum_NMEA(msg, len, &checkSum);
		SetMsgSum_NMEA(msg, len, checkSum);
		return 0;
	}
}

/**
  *@brief	
  */
int32_t SetLeap_LSF(char *msg, leapSecond_t *leap)
{
	uint32_t len;
	char *lsfType;
	char leapCurr[10] = {'\0'};
	char leapNext[10] = {'\0'};
	char leapWeek[4] = {'\0'};
	char leapDayOfWeek[2] = {'\0'};
	int32_t sysLeapOffset;
	uint32_t dayOfWeek;
	uint16_t sysWeek;

	len = strlen(msg);

	if (GetMsgType_NMEA(msg, len) == LSF) {
		GetMsgElement_NMEA(msg, len, 1, &lsfType);
		if (lsfType[0] == '0') {  // GPS LSF frame
			sysLeapOffset = leapSecond.currentLeap - 9;
			sysWeek = GetSysWeek_Gps(&(leap->leapOccur));
			dayOfWeek = GetDayOfWeek_Gps(&(leap->leapOccur));
		}
		else if (lsfType[0] == '1') {  // BD LSF frame
			sysLeapOffset = leapSecond.currentLeap - 23;
			sysWeek = GetSysWeek_Bd(&(leap->leapOccur));
			dayOfWeek = GetDayOfWeek_Bd(&(leap->leapOccur));
		}
		else {
			return -1;
		}

		sprintf(leapCurr, "%" PRIi32, sysLeapOffset);
		SetMsgElement_NMEA(msg, len, 3, leapCurr);
		sprintf(leapNext, "%" PRIi32, (sysLeapOffset + leapSecond.leapDirect));
		SetMsgElement_NMEA(msg, len, 4, leapNext);
		sprintf(leapWeek, "%d", (sysWeek % 256));
		SetMsgElement_NMEA(msg, len, 8, leapWeek);
		sprintf(leapDayOfWeek, "%d", dayOfWeek);
		SetMsgElement_NMEA(msg, len, 7, leapDayOfWeek);
		return 0;
	}
	else {
		return -1;
	}
}

/**
  *@brief
  */
int32_t ParseLeap_LSF(char *msg, leapSecond_t *leap)
{
	uint32_t len, size;
	char *lsfType, *leapCurr_Ptr, *leapNext_Ptr, *leapWeek_Ptr, *leapDayOfWeek_Ptr;
	char leapCurr_buff[10] = {'\0'};
	char leapNext_buff[10] = {'\0'};
	char leapWeek_buff[4] = {'\0'};
	char leapDayOfWeek_buff[2] = {'\0'};
	int16_t leapCurr, leapNext;
	int32_t sysLeapOffset;
	uint32_t dayOfWeek;
	uint16_t sysWeek;

	len = strlen(msg);
	if (GetMsgType_NMEA(msg, len) == LSF) {
		GetMsgElement_NMEA(msg, len, 1, &lsfType);
		if (lsfType[0] == '0') {  // GPS LSF frame
			size = GetMsgElement_NMEA(msg, len, 3, &(leapCurr_Ptr));
			strncpy(leapCurr_buff, leapCurr_Ptr, size);
			size = GetMsgElement_NMEA(msg, len, 4, &(leapNext_Ptr));
			strncpy(leapNext_buff, leapNext_Ptr, size);
			size = GetMsgElement_NMEA(msg, len, 7, &(leapDayOfWeek_Ptr));
			strncpy(leapDayOfWeek_buff, leapDayOfWeek_Ptr, size);
			size = GetMsgElement_NMEA(msg, len, 8, &(leapWeek_Ptr));
			strncpy(leapWeek_buff, leapWeek_Ptr, size);

			sscanf(leapCurr_buff, "%" SCNd16 "", &(leapCurr));
			sscanf(leapNext_buff, "%" SCNd16 "", &(leapNext));
			sscanf(leapDayOfWeek_buff, "%" SCNu32 "", &(dayOfWeek));
			sscanf(leapWeek_buff, "%" SCNu16 "", &(sysWeek));
		}
		else if (lsfType[0] == '1') {  // BD LSF frame
			return 0;
		}
		else {
			return -1;
		}
		leap->currentLeap = leapCurr + 9;
		leap->leapDirect = leapNext - leapCurr;
		/**@todo parse of date not finished
		  *to be continue...
		  */
		return 0;
	}
	else {
		return -1;
	}
}

/**
  *@brief
  */
int32_t SetLeap_LPS(char *msg, leapSecond_t *leap)
{
	uint32_t len;
	int16_t leapStep, leapDirect, totalLeap;
	char leapStep_buff[2] = {'\0'};
	char leapDirect_buff[2] = {'\0'};
	char totalLeap_buff[5] = {'\0'};

	len = strlen(msg);
	if (GetMsgType_NMEA(msg, len) == LPS) {
		leapStep = (leap->leapDirect > 0) ? (leap->leapDirect) : (-(leap->leapDirect));
		leapDirect = (leap->leapDirect < 0) ? 1 : 0;
		totalLeap = leap->currentLeap;

		sprintf(leapStep_buff, "%01d", leapStep);
		sprintf(leapDirect_buff, "%01d", leapDirect);
		sprintf(totalLeap_buff, "%03d", totalLeap);

		SetMsgElement_NMEA(msg, len, 1, leapStep_buff);
		SetMsgElement_NMEA(msg, len, 2, leapDirect_buff);
		SetMsgElement_NMEA(msg, len, 3, totalLeap_buff);
		return 0;
	}
	else {
		return -1;
	}
}

/**
  *@brief
  */
int32_t ParseLeap_LPS(char *msg, leapSecond_t *leap, localTime_t *time)
{
	uint32_t len, size;
	int16_t leapStep, leapDirect, totalLeap;
	localTime_t tmpTime;
	char *leapStep_Ptr, *leapDirect_Ptr, *totalLeap_Ptr;
	char leapStep_buff[3] = {'\0'};
	char leapDirect_buff[3] = {'\0'};
	char totalLeap_buff[5] = {'\0'};

	memcpy(&tmpTime, time, sizeof(localTime_t));
	len = strlen(msg);
	if (GetMsgType_NMEA(msg, len) == LPS) {
		if (strcmp(msg, lastLPS) == 0) {
			return 0;
		}
		else {
			AdjustTimezone(&tmpTime, -(tmpTime.timeZone));
			strcpy(lastLPS, msg);
			size = GetMsgElement_NMEA(msg, len, 1, &leapStep_Ptr);
			strncpy(leapStep_buff, leapStep_Ptr, size);
			size = GetMsgElement_NMEA(msg, len, 2, &leapDirect_Ptr);
			strncpy(leapDirect_buff, leapDirect_Ptr, size);
			size = GetMsgElement_NMEA(msg, len, 3, &totalLeap_Ptr);
			strncpy(totalLeap_buff, totalLeap_Ptr, size);

			sscanf(leapStep_buff, "%" SCNd16 "", &leapStep);
			sscanf(leapDirect_buff, "%" SCNd16 "", &leapDirect);
			sscanf(totalLeap_buff, "%" SCNd16 "", &totalLeap);

			leap->leapDirect = (leapDirect == 0) ? (leapStep) : (-leapStep);
			leap->currentLeap = totalLeap;
			leap->leapOccur.utc_Year = tmpTime.utc_Year;
			leap->leapOccur.utc_Month = (tmpTime.utc_Month > 7) ? 12 : 6;
			leap->leapOccur.utc_Day = (tmpTime.utc_Month > 7) ? 31 : 30;
			leap->leapOccur.utc_Hour = 23;
			leap->leapOccur.utc_Minute = 59;
			leap->leapOccur.utc_Second = (leapDirect == 0) ? 60 : 59;
			leap->leapOccur.leaps = leap->currentLeap + leap->leapDirect;
			leap->leapOccur.timeZone = 0;
			UtcToTai(&(leap->leapOccur));
			//			time->leaps = leap->currentLeap;
			//			UtcToTai(time);
			AdjustTimezone(&leap->leapOccur, tmpTime.timeZone);
			return 0;
		}
	}
	else {
		return -1;
	}
}

/**
 *@brief	Parse NMEA-0183 v4.1 xxRMC message
 *@param	msg		message string
 *@param	len		length of message string
 *@param	*time	localTime_t time structure
 *@retval	-1		Failed to parse message or no legal element in message
 *@retval	0		Parsing successfully and data is trustable
 *@retval	1		Parsing successfully and data is untrustable
 */

int32_t ParseTime_RMC(char *msg, uint32_t len, localTime_t *time)
{
	char *vernier;
	uint32_t elem_len;
	int16_t hour, minute, second, month, day;
	int32_t year;
	char status;
	elem_len = GetMsgElement_NMEA(msg, len, 1, &vernier);
	if ((elem_len < 9) || (vernier == NULL)) {
		return -1;
	}
	else {
		sscanf(vernier, "%2" SCNd16 "%2" SCNd16 "%2" SCNd16, &hour, &minute, &second);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 9, &vernier);
	if ((elem_len < 6) || (vernier == NULL)) {
		return -1;
	}
	else {
		sscanf(vernier, "%2" SCNd16 "%2" SCNd16 "%2" SCNd32, &day, &month, &year);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 2, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		status = vernier[0];
	}
	time->utc_Year = year + currCentury;
	time->utc_Month = month;
	time->utc_Day = day;
	time->utc_Hour = hour;
	time->utc_Minute = minute;
	time->utc_Second = second;
	time->timeZone = 0;
	UtcToTai(time);
	if ((status == 'A') || (status == 'D')) {
		return 0;
	}
	else {
		return 1;
	}
}

/**
 *@brief	Parse effective satellite number from NMEA-0183 v4.1 GSA message
 *@param	msg		message string
 *@param	len		message length
 *@param	*sysid	return the system identifier of the message.
 *					1 for GPS, 2 for GLONASS, 3 for Galileo, 4 for Beidou
 *@retval	>0	Effective satellite number
 *@retval	-1	Error when parsing the message
 *@retval	-2	Unrecognized system id in the message
 */

int32_t ParseSatNum_GSA(char *msg, uint32_t len, int32_t *sysid)
{
	uint16_t elem_index;
	char *vernier;
	int16_t elem_len;
	char sysid_buf;
	elem_len = GetMsgElement_NMEA(msg, len, 18, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		sysid_buf = *vernier;
		if (isdigit(sysid_buf) == 0) {
			return -2;
		}
		else {
			*sysid = sysid_buf - '0';
		}
	}
	for (elem_index = 3; elem_index < 15; elem_index++) {
		elem_len = GetMsgElement_NMEA(msg, len, elem_index, &vernier);
		if ((elem_len == 0) || (vernier == NULL)) {
			break;
		}
	}
	return elem_index - 3;
}

/**
 *@brief	Parse location data from NMEA-0183 v4.1 GNS message
 *@param	msg			message string
 *@param	len			message length
 *@param	*latitude	return the latitude data
 *@param	*longitude	return the longitude data
 *@param	*altitude	return the altitude data
 *@retval	0	successfully parsed the message
 *@retval	-1	Error when parsing the message
 *@retval	-2	Unrecognized indicator
 */

int32_t ParseLocation_GNS(char *msg, uint32_t len, double *latitude, double *longitude, double *altitude)
{
	char *vernier;
	int16_t elem_len;
	char elem_buf[12];
	int32_t lat_deg, lat_min_int, lat_min_frac;
	int32_t lon_deg, lon_min_int, lon_min_frac;
	double alt_db, sep_db;

	elem_len = GetMsgElement_NMEA(msg, len, 2, &vernier);
	if ((elem_len < 10) || (vernier == NULL)) {
		return -1;
	}
	else {
		memcpy(elem_buf, vernier, elem_len);
		elem_buf[elem_len] = '\0';
		sscanf(elem_buf, "%02" SCNd32 "%02" SCNd32 ".%05" SCNd32, &lat_deg, &lat_min_int, &lat_min_frac);
		lat_min_int = (lat_min_int * 100000) + lat_min_frac;
		*latitude = ((double)lat_deg) + ((double)lat_min_int / 6000000);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 3, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		switch (vernier[0]) {
		case 'N':
			break;
		case 'S':
			*latitude = -(*latitude);
			break;
		default:
			return -2;
			break;
		}
	}
	elem_len = GetMsgElement_NMEA(msg, len, 4, &vernier);
	if ((elem_len < 11) || (vernier == NULL)) {
		return -1;
	}
	else {
		memcpy(elem_buf, vernier, elem_len);
		elem_buf[elem_len] = '\0';
		sscanf(elem_buf, "%03" SCNd32 "%02" SCNd32 ".%05" SCNd32, &lon_deg, &lon_min_int, &lon_min_frac);
		lon_min_int = (lon_min_int * 100000) + lon_min_frac;
		*longitude = ((double)lon_deg) + ((double)lon_min_int / 6000000);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 5, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		switch (vernier[0]) {
		case 'E':
			break;
		case 'W':
			*longitude = -(*longitude);
			break;
		default:
			return -2;
			break;
		}
	}
	elem_len = GetMsgElement_NMEA(msg, len, 9, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		memcpy(elem_buf, vernier, elem_len);
		elem_buf[elem_len] = '\0';
		sscanf(elem_buf, "%lf", &alt_db);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 10, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		memcpy(elem_buf, vernier, elem_len);
		elem_buf[elem_len] = '\0';
		sscanf(elem_buf, "%lf", &sep_db);
	}
	*altitude = (alt_db + sep_db);
	return 0;
}

int32_t ParseCentury_ZDA(char *msg, uint32_t len)
{
	char *vernier;
	int16_t elem_len;
	char elem_buf[10];
	int32_t tmp_century;

	elem_len = GetMsgElement_NMEA(msg, len, 4, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	memcpy(elem_buf, vernier, elem_len);
	elem_buf[elem_len] = '\0';
	sscanf(elem_buf, "%" SCNd32, &tmp_century);
	currCentury = tmp_century / 100;
	currCentury *= 100;
	return 0;
}

/**
 *@brief	Parse NMEA-0183 v4.1 xxZDA message
 *@param	msg		message string
 *@param	len		length of message string
 *@param	*time	localTime_t time structure
 *@retval	-1		Failed to parse message or no legal element in message
 *@retval	0		Parsing successfully
 */

int32_t ParseTime_ZDA(char *msg, uint32_t len, localTime_t *time)
{
	char *vernier;
	uint32_t elem_len;
	int16_t hour, minute, second, month, day;
	int32_t year;

	elem_len = GetMsgElement_NMEA(msg, len, 1, &vernier);
	if ((elem_len < 9) || (vernier == NULL)) {
		return -1;
	}
	else {
		sscanf(vernier, "%2" SCNd16 "%2" SCNd16 "%2" SCNd16, &hour, &minute, &second);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 2, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		sscanf(vernier, "%2" SCNd16 "", &day);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 3, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		sscanf(vernier, "%2" SCNd16 "", &month);
	}
	elem_len = GetMsgElement_NMEA(msg, len, 4, &vernier);
	if ((elem_len < 1) || (vernier == NULL)) {
		return -1;
	}
	else {
		sscanf(vernier, "%4" SCNd32 "" , &year);
	}
	time->utc_Year = year;
	time->utc_Month = month;
	time->utc_Day = day;
	time->utc_Hour = hour;
	time->utc_Minute = minute;
	time->utc_Second = second;
	time->timeZone = 0;
	UtcToTai(time);
	return 0;
}
