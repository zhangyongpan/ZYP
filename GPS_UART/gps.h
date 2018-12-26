#ifndef __GPS_H_
#define __GPS_H_

typedef unsigned int UINT;
typedef int BYTE;
typedef long int WORD;

typedef struct __gpsrmc__ {
	UINT time;			//格林威治时间
	char pos_state;		//定位状态
	float latitude;		//纬度
	float longitude;	//经度
	float speed;		//移动速度
	float direction;	//方向
	UINT date;			//日期
	float declination;  //磁偏角
	char dd;			//磁偏角方向
	char mode;
} GPRMC;
extern int gps_analysis(char *buff, GPRMC *gps_data);
extern int print(GPRMC *gps_data);
extern int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop);

#endif