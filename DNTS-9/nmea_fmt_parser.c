/**
  *@file	nmea_fmt_parser.c
  *@brief	NMEA-0183 message process support
  *@author	Eggcar
  */

#include "nmea_fmt_parser.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

/**
  *@brief	Check if the message format is legal.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@retval	Error type. 
  *			0 for legal, 
  *			-1 if start character is not '$', 
  *			-2 if length larger than MAX_MESSAGE_LENGTH_NMEA
  */
int8_t IfLegal_NMEA(const char *msg, uint32_t len)
{
	if (msg[0] != '$') {
		return -1;
	}
	else if(len > MAX_MESSAGE_LENGTH_NMEA) {
		return -2;
	}
	else {
		return 0;
	}
}

/**
  *@brief	Get the message type of the unicore Module.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@retval	Enumeration of the message, 
  *			UNDEFINED if type not recognized.
  */
msgType_NMEA_t GetMsgType_NMEA(char *msg, uint32_t len)
{
	char tmp[MAX_TYPE_LENGTH_NMEA + 1] = {'\0'};
	int16_t location;

	if (IfLegal_NMEA(msg, len) != 0) {
		return UNDEFINED;
	}
	else {
		location = LocateMsgElement_NMEA(msg, len, 1);
		if (GetSysId_NMEA(msg, len) == NONE) {
			if (location == -1) {
                if (len > 31) {
                    return UNDEFINED;
                }
                else if (len < 2) {
                	return UNDEFINED;
                }
                else {
                    strncpy(tmp, (msg + 1), len - 1);
                }
			}
			else {
				if (location > 33) {
					return UNDEFINED;
				}
				else if (location < 3) {
					return UNDEFINED;
				}
				else if (msg[location] == ',') {
						strncpy(tmp, (msg + 1), location - 1);
				}
				else {
					strncpy(tmp, (msg + 1),  location - 2);
				}
			}
		}
		else if (location == -1) {
            if (len > 33) {
                return UNDEFINED;
            }
            else if (len < 4) {
            	return UNDEFINED;
            }
            else {
                strncpy(tmp, (msg + 3), len - 3);
            }
		}
		else {
			if (location > 33) {
				return UNDEFINED;
			}
			else if (location < 5) {
				return UNDEFINED;
			}
			else if (msg[location] == ',') {
				strncpy(tmp, (msg + 3), location - 3);
			}
			else {
				strncpy(tmp, (msg + 3),  location - 4);
			}
		}
/*------------------------------------------------------------------*/
		if (strcmp(tmp, "GGA") == 0) {
			return GGA;
		}
		else if (strcmp(tmp, "GLL") == 0) {
			return GLL;
		}
		else if (strcmp(tmp, "GSA") == 0) {
			return GSA;
		}
		else if (strcmp(tmp, "GSV") == 0) {
			return GSV;
		}
		else if (strcmp(tmp, "RMC") == 0) {
			return RMC;
		}
		else if (strcmp(tmp, "VTG") == 0) {
			return VTG;
		}
		else if (strcmp(tmp, "ZDA") == 0) {
			return ZDA;
		}
		else if (strcmp(tmp, "GST") == 0) {
			return GST;
		}
		else if (strcmp(tmp, "GNS") == 0) {
			return GNS;
		}
		else if (strcmp(tmp, "NAVPOS") == 0) {
			return NAVPOS;
		}
		else if (strcmp(tmp, "NAVVEL") == 0) {
			return NAVVEL;
		}
		else if (strcmp(tmp, "NAVTIME") == 0) {
			return NAVTIME;
		}
		else if (strcmp(tmp, "NAVACC") == 0) {
			return NAVACC;
		}
		else if (strcmp(tmp, "ANTSTAT") == 0) {
			return ANTSTAT;
		}
		else if (strcmp(tmp, "TIMTP") == 0) {
			return TIMTP;
		}
		else if (strcmp(tmp, "TIMEM") == 0) {
			return TIMEM;
		}
		else if (strcmp(tmp, "LSF") == 0) {
			return LSF;
		}
		else if (strcmp(tmp, "LPS") == 0) {
			return LPS;
		}
		else if (strcmp(tmp, "SIGNAL") == 0) {
			return SIGNAL;
		}
		else if(strcmp(tmp,"IPADD") == 0)
		{
			return IPADD;
		}
		else {
			return UNDEFINED;
		}
/*------------------------------------------------------------------*/
	}
}

/**
  *@brief	Get the satellite system identifier of the message.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@retval	Enumeration of the system identifier, NONE if no sysId in the message.
  */
sysIdentifier_NMEA_t GetSysId_NMEA(char *msg, uint32_t len)
{
	char tmp[10] = { '\0' };
	strncpy(tmp, (msg + 1), 2);
	if (strcmp(tmp, "GP") == 0) {
		return GP;
	}
	else if (strcmp(tmp, "BD") == 0) {
		return BD;
	}
	else if (strcmp(tmp, "GB") == 0) {
		return GB;
	}
	else if (strcmp(tmp, "GL") == 0) {
		return GL;
	}
	else if (strcmp(tmp, "GA") == 0) {
		return GA;
	}
	else if (strcmp(tmp, "GN") == 0) {
		return GN;
	}
	else {
		return NONE;
	}
}

/**
  *@brief	Check if the message contains a checksum.
  *@param	*msg	array of the message
  *@param	len		total length of the message
  *@retval	0 if message contains a checksum
  *			-1 if message contains no checksum
  */
int8_t ContainMsgSum_NMEA(const char *msg, uint32_t len)
{
	char *tmp = NULL;
	tmp = strchr(msg, '*');
	if (tmp != NULL) {
		return 0;
	}
	else {
		return -1;
	}
}

/**
  *@brief	Calculate message checksum of the Unicore Module.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@param	*chksum	container of checksum result
  *@retval	Error type.
  *			0 for check ok,
  *			-1 for check error,
  *			-2 for illegal checksum format
  */
int8_t CalcMsgSum_NMEA(const char *msg, uint32_t len, uint16_t *chksum)
{
	char tmp[MAX_MESSAGE_LENGTH_NMEA] = { '\0' };
	char *firstStar = NULL;
	uint16_t checkSum = 0;
	firstStar = strchr(msg, (int)'*');
	if ((firstStar == NULL) || \
		((firstStar - msg) != (len - 3)) || \
		(ContainMsgSum_NMEA(msg, len) == -1)) {
		return -2;
	}
	else {
		strncpy(tmp, (msg + 1), (firstStar - msg - 1));
		checkSum = tmp[0];
		for (int i = 1; i < strlen(tmp); i++) {
			checkSum ^= tmp[i];
		}
		*chksum = checkSum;
		return 0;
	}
}

/**
  *@brief	Set checksum to the end of the message
  *			'*' must exist at the right place of the message
  *			This function locate '*' to write checksum
  *			without check of legality.
  *@param	*msg		array of the message string
  *@param	len			total length of the message
  *@param	checkSum	16-bit checksum to be write
  *@retval	Error type.
  *			0 for success,
  *			-2 for error checksum format
  */
int8_t SetMsgSum_NMEA(char *msg, uint32_t len, uint16_t checkSum)
{
	char *starLocation = NULL;
	char chksumBuff[3] = { '\0' };
	starLocation = strchr(msg, '*');
	if (starLocation == NULL) {
		return -2;
	}
	else {
		sprintf(chksumBuff, "%02"PRIX16"", checkSum);
		strncpy(starLocation + 1, chksumBuff, 2);
		return 0;
	}
}

/**
  *@brief	Check message checksum of the NMEA message.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@retval	Error type. 
  *			0 for check ok, 
  *			-1 for check error,
  *			-2 for illegal checksum format
  */
int8_t CheckMsgSum_NMEA(const char *msg, uint32_t len)
{
	char tmp[MAX_MESSAGE_LENGTH_NMEA] = { '\0' };
	char *firstStar = NULL;
	char msgChkSum[3] = { '\0' };
	uint16_t checkSum = 0, checkSumComp = 0;
	char offset;

	firstStar = strchr(msg, (int)'*');
	if ((firstStar == NULL) || \
		((firstStar - msg) != (len - 3)) || \
		(ContainMsgSum_NMEA(msg, len) == -1)) {
		return -2;
	}
	strncpy(tmp, (msg + 1), (firstStar - msg - 1));
	checkSum = tmp[0];
	for (int i = 1; i < strlen(tmp); i++) {
		checkSum ^= tmp[i];
	}

	strncpy(msgChkSum, firstStar + 1, 2);
	for (int i = 0; i < 2; i++) {
		msgChkSum[i] = toupper(msgChkSum[i]);
		if ((msgChkSum[i] <= '9') && (msgChkSum[i] >= '0')) {
			offset = '0';
		}
		else if ((msgChkSum[i] <= 'F') && (msgChkSum[i] >= 'A')) {
			offset = 'A' - 10;
		}
		else {
			return -2;
		}
		checkSumComp |= (((int)msgChkSum) - offset) << (i << 3);
	}

	if (checkSum == checkSumComp) {
		return 0;
	}
	else {
		return -1;
	}
}

/**
  *@brief	Count total elements in the message,include message type and checksum.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@retval	Number of elements in the message
  */
int16_t CountMsgElement_NMEA(const char *msg, uint32_t len) 
{
	int16_t count = 0;
	int8_t err;
	err = IfLegal_NMEA(msg, len);
	if (err == 0) {
		count = 1;
		for (int i = 0; i < len; i++) {
			if ((msg[i] == ',') || (msg[i] == '*')) {
				count++;
			}
			else {
				//continue
			}
		}
		return count;
	}
	else {
		return err;
	}
}

/**
  *@brief	Get the particular element of the message.
  *			The index of the first element after the message type is 1(one).
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@param	index	the index number of the particular element
  *@param	**elem	pointer to the first character of the element. Set to NULL if element not found
  *@retval	Length of the element. 
  *			-1 if element not found
  *@warning	DO MAKE SURE THAT *msg AND *elem ARE NOT OVERLAPPED
  *			This function DOES NOT check the legality of *elem
  */
int16_t GetMsgElement_NMEA(char *msg, uint32_t len, uint16_t index, char **elem)
{
	int16_t location, locationNext;
	int16_t size;

	location = LocateMsgElement_NMEA(msg, len, index);
	if (location == -1) {
		*elem = NULL;
		return -1;
	}
	else {
		*elem = &(msg[location]);
		locationNext = LocateMsgElement_NMEA(msg, len, index + 1);

		if (**elem == ',') {
//			*elem = NULL;
			return 0;
		}
		else {
			if (locationNext == -1) {
				size = len - location;
			}
			else if (msg[locationNext] == ',') {
				size = locationNext - location;
			}
			else {
				size = locationNext - location - 1;
			}
			return size;
		}
	}
}

/**
  *@brief	Set the particular element of the message.
  *			If index overflowed, empty elements will be added automatically
  *			The index of the first element after the message type is 1(one).
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@param	index	the index number of the particular element
  *@param	*element	the element to be set
  *@retval	Error type. 0 for success, 
  *			-1 if index out of the message, 
  *			-2 if message longer than MAX_MESSAGE_LENGTH_NMEA.
  */
int8_t SetMsgElement_NMEA(char *msg, uint32_t len, uint16_t index, const char *element)
{
	int16_t location, locationNext;
	int16_t elemCount;
	uint16_t elemLen, elemLen_Prim;
	char *tmp;
	char buff[MAX_MESSAGE_LENGTH_NMEA] = { '\0' };

	elemLen = strlen(element);
	elemCount = CountMsgElement_NMEA(msg, len);

	if (index < elemCount) {
		elemLen_Prim = GetMsgElement_NMEA(msg, len, index, &tmp);
	}
	else {
		for (; elemCount <= index; elemCount++) {
			msg[++len] = ',';
		}
		elemLen_Prim = 0;
	}
	
	if ((len - elemLen_Prim + elemLen) > MAX_MESSAGE_LENGTH_NMEA) {
		return -2;
	}
	else { // if message length after insertion less than MAX_MESSAGE_LENGTH_NMEA
		location = LocateMsgElement_NMEA(msg, len, index);
		locationNext = LocateMsgElement_NMEA(msg, len, index + 1);
		locationNext = (locationNext < 0) ? (len): (locationNext);
		if(msg[locationNext] == ',') {
			locationNext++;
		}
		else {
		}
		strncpy(buff, &msg[locationNext - 1], len - locationNext + 1);	// Put the elements after index into buffer
		if (msg[location] == ',') {
			location++;
		}
		memset(&msg[location], '\0', len - location);					// Clear elements after index
		strncpy(&msg[location], element, elemLen);						// Write THE element into message
		strncpy(&msg[location + elemLen], buff, len - locationNext + 1);
		return 0;
	}
}

/**
  *@brief	Locate the first character of the particular element in the message.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@param	index	the index number of the particular element
  *@retval	Location of the first character of the element. 
  *			-1 if index out of the message.
  */
int16_t LocateMsgElement_NMEA(char *msg, uint32_t len, uint16_t index)
{
	char *vernier = NULL;
	char *tmp = NULL;

	vernier = &(msg[1]);
	for (int i = 0; i < index; i++) {
		tmp = strchr(vernier, ',');
		if (tmp == NULL) {
			tmp = strchr(vernier, '*');
			if (tmp == NULL) {
				return -1;
			}
			else { 
				//here run to vernier = tmp + 1;
			}
		}
		else {  
			//here run to vernier = tmp + 1;
		}
		vernier = tmp + 1;
	}
	if (*vernier == ',') {
		vernier--;
	}
	return (vernier - msg);
}

/**
  *@brief	Insert an element to the message.
  *@param	*msg	array of the message string
  *@param	len		total length of the message
  *@param	index	the index of the inserted element
  *@param	element	the element to be inserted
  *@retval	Location of the first character of the element. 
  *			-2 if message larger than MAX_MESSAGE_LENGTH_NMEA.
  *@todo	InsertMsgElement_NEMA not finished.
  */
int16_t InsertMsgElement_NMEA(char *msg, uint32_t len, uint16_t idnex, char* element)
{
	return -10;
}
