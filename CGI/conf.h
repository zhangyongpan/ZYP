#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

typedef struct
{
	unsigned char version[5];
	unsigned char Device_sn[8];

	unsigned char ip1[4];  //ǧ��
	unsigned char gateway1[4];
	unsigned char mask1[4];

	unsigned char debug_ip[4];  //����
	unsigned char debug_gateway[4];
	unsigned char debug_mask[4];

	char password[12];
	unsigned int sat_num[3];  //依次为GPS、beidou、Glonass
	char tx_sta1[10];		  //两个接收机板 天线状态： a：天线正常 o: 天线开路  s:天线短路 u:状态未知
	char tx_sta2[10];
	char osstate[10];  //频率源状态:  f: 自由振荡 h:保持 t: 追踪 l: 锁定 f: 自检失败

} NTSSetting;  //gm player setting

typedef struct {
	char *ssi_name;
	void (*ssi_fn)(int para);
	int para;
} ssi_types;  //��ҳ���ݵı�����Ӧ�Ĵ������
