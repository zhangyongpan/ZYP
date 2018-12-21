#define CONFIG_FILE "/opt/sysfile/syscfg.cfg"
#define IPDEVICE "/dev/ttyO4"
#define SERIALDEVICE "/dev/ttyO2"
#define NUM_FILE "/opt/sysfile/num.cfg"
#include "nmea_fmt_parser.h"
typedef struct
{
	unsigned char version[5];
	unsigned char Device_sn[8];

	unsigned char ip1[4];  //网卡2
	unsigned char gateway1[4];
	unsigned char mask1[4];

	unsigned char debug_ip[4];  //网卡1
	unsigned char debug_gateway[4];
	unsigned char debug_mask[4];

	char	password[12];
	unsigned int sat_num[3]; //依次为GPS、beidou、Glonass
	char tx_sta1[10];          //两个接收机板 天线状态： a：天线正常 o: 天线开路  s:天线短路 u:状态未知 
	char tx_sta2[10];
	char osstate[10];          //频率源状态:  f: 自由振荡 h:保持 t: 追踪 l: 锁定 f: 自检失败


} NTSSetting;

struct msg_info {
	long mytype;
	int i;
	char s[20];
};
