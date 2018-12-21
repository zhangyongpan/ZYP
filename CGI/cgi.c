#include "conf.h"

#define NO_USER 0
#define GUEST 1
#define ADMIN 2
#define SUPER 3
//#define ROOT        4
//#define NULL_	 5
#define BUFFER_SIZE 1024

#define CMD_CONFIG 0
#define CMD_REBOOT 1
#define CMD_PLAYLIST 2
#define CMD_NORMAL 3

unsigned char login = 0, sets = 0, user = NO_USER;
char name[15], stdin_data[10240];
NTSSetting settings;

int get_config(char *filename)  //根据路径下的配置文件来对系统进行配置，初始化
{
	int fd;

	if ((fd = open(filename, O_RDONLY)) < 0)
		return -1;
	if (read(fd, &settings, sizeof(NTSSetting)) != sizeof(NTSSetting)) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int set_config(char *filename)  //配置文件保存
{
	int fd;

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC)) < 0)
		return -1;
	if (write(fd, &settings, sizeof(NTSSetting)) != sizeof(NTSSetting)) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int get_pid()  //获取main进程的进程ID，这个进程ID是由main启动的时候写入文件的，只写一次
{
	FILE *fp;
	int pid;
	fp = fopen("/opt/rd/mainpid", "r");
	if (!fp)
		exit(1);
	fscanf(fp, "%d", &pid);
	fclose(fp);
	return pid;
}

void send_signal(int sig)  //向main进程发送指定信号
{
	int pid = 0;
	char cmd[20];
	pid = get_pid();
	sprintf(cmd, "kill -%d %d", sig, pid);
	system(cmd);
}

int send_cmd(int cmd, char *par)
{
	int fd;
	char cmdstr[30];

	if ((fd = open("/opt/rd/cmd", O_WRONLY | O_CREAT | O_TRUNC | O_SYNC)) < 0)
		return -1;
	memset(cmdstr, 0, sizeof(cmdstr));
	sprintf(cmdstr, "%d %s", cmd, par);
	if (write(fd, cmdstr, strlen(cmdstr)) != strlen(cmdstr)) {
		close(fd);
		return -1;
	}
	close(fd);
	send_signal(SIGUSR1);
	return 0;
}

void restart()
{
	send_cmd(CMD_REBOOT, NULL);
}

void put_setting()  //settings.htm对应的显示程序
{
	switch (sets) {
	case 0:
		printf("%s", settings.Device_sn);
		break;
	case 1:
		printf("%s", settings.version);
		break;
	case 2:
		printf("%d", settings.sat_num[0]);  //GPS
		break;
	case 3:
		printf("%d", settings.sat_num[1]);  //beidou
		break;
	case 4:
		printf("%d", settings.sat_num[2]);  //Glonass
		break;
	case 5:
		if (strcmp(settings.tx_sta1, "active") == 0)  //卫星天线1状态
		{
			printf("天线正常");
		}
		else if (strcmp(settings.tx_sta1, "open") == 0) {
			printf("天线开路");
		}
		else if (strcmp(settings.tx_sta1, "short") == 0) {
			printf("天线短路");
		}
		else {
		}

		break;
	case 6:
		if (strcmp(settings.tx_sta2, "active") == 0)  //卫星天线1状态
		{
			printf("天线正常");
		}
		else if (strcmp(settings.tx_sta2, "open") == 0) {
			printf("天线开路");
		}
		else if (strcmp(settings.tx_sta2, "short") == 0) {
			printf("天线短路");
		}
		else {
		}
		break;
	case 7:
		if (strcmp(settings.osstate, "freerun") == 0) {  //频率源状态
			printf("自由振荡");
		}
		else if (strcmp(settings.osstate, "holdover") == 0) {
			printf("保持");
		}
		else if (strcmp(settings.osstate, "tracking") == 0) {
			printf("追踪");
		}
		else if (strcmp(settings.osstate, "lock") == 0) {
			printf("锁定");
		}
		else if (strcmp(settings.osstate, "failure") == 0) {
			printf("自检失败");
		}
		break;
	case 8:
		printf(" ");
		break;
		/*		
	case 8: {
		time_t timep;
		struct tm *p;
		int year;

		time(&timep);
		p = localtime(&timep);
		year = p->tm_year;
		if (year < 1900)
			year += 1900;
		printf("%d-%02d-%02d %02d:%02d:%02d  ", year, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	} break;
	*/
	default:
		break;
	}
}

void put_paras()  //paras.htm显示程序
{
	switch (sets) {
	case 0:
		printf("%d.%d.%d.%d", settings.ip1[0], settings.ip1[1], settings.ip1[2], settings.ip1[3]);
		break;
	case 1:
		printf("%d.%d.%d.%d", settings.gateway1[0], settings.gateway1[1], settings.gateway1[2], settings.gateway1[3]);
		break;
	case 2:
		printf("%d.%d.%d.%d", settings.mask1[0], settings.mask1[1], settings.mask1[2], settings.mask1[3]);
		break;
	case 3:
		printf("%d.%d.%d.%d", settings.debug_ip[0], settings.debug_ip[1], settings.debug_ip[2], settings.debug_ip[3]);
		break;
	case 4:
		printf("%d.%d.%d.%d", settings.debug_gateway[0], settings.debug_gateway[1], settings.debug_gateway[2], settings.debug_gateway[3]);
		break;
	case 5:
		printf("%d.%d.%d.%d", settings.debug_mask[0], settings.debug_mask[1], settings.debug_mask[2], settings.debug_mask[3]);
		break;
	default:
		break;
	}
}

void sh_paras(int para)  //判断结构体ssi_types中传递给成员函数void (*ssi_fn)(int para)的参数para
{
	switch (para) {
	case 1:  //settings.htm
		put_setting();
		sets++;
		if (sets >= 9)
			sets = 0;
		break;
	case 2:  //paras.htm
		put_paras();
		sets++;
		if (sets >= 6)
			sets = 0;
		break;
	default:
		break;
	}
}

static ssi_types ssi_list[] = {  //参数处理结构
	{"setting", sh_paras, 1},
	{"paras", sh_paras, 2}};

int put_htm_file(char *filename)  //输出静态网页
{
	FILE *fp;
	int nFileLen, nCount;
	char pathname[30];
	unsigned char trans_buffer[BUFFER_SIZE];

	printf("content-type: text/html\n\n");

	if (login)
		printf("Set-Cookie: %s\n\n", name);  //设置cookie

	sprintf(pathname, "/opt/goahead/web/pages/%s", filename);
	fp = fopen(pathname, "rb");
	if (!fp)
		return -1;
	fseek(fp, 0L, SEEK_END);
	nFileLen = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	do {
		memset(trans_buffer, 0, sizeof(trans_buffer));
		nCount = fread(trans_buffer, 1, sizeof(trans_buffer), fp);
		fwrite(trans_buffer, nCount, 1, stdout);
	} while (nCount > 0 && !feof(fp));
	fclose(fp);
	return 0;
}

void action(char *act)  //根据传入的网页关键字（例如“io_set”）查找处理程序，并传递处理参数para
{
	ssi_types *ssi;
	int para = 0;
	for (ssi = ssi_list; ssi->ssi_name != NULL; ssi++) {
		if (!strcmp(ssi->ssi_name, act)) {
			para = ssi->para;
			ssi->ssi_fn(para);
			break;
		}
	}
}

int put_sht_file(char *filename)  //输出动态网页
{
	FILE *fp;
	int nCount = 0;
	char pathname[50], act[10], ch, getflag = 0, puted = 0, getat = 0;
	int i = 0;
	char trans_buffer[BUFFER_SIZE];

	printf("content-type: text/html\n\n");
	sprintf(pathname, "/opt/goahead/web/pages/%s", filename);
	fp = fopen(pathname, "rb");
	if (!fp)
		return -1;
	do {
		ch = fgetc(fp);
		if (!getat && !puted)  //查找“#”中间的网页关键字
		{
			if (getflag && (ch == '#'))  //连续两个"#"
			{
				trans_buffer[nCount] = 0;
				fwrite(trans_buffer, sizeof(char), nCount, stdout);
				nCount = 0;
				puted = 1;
				getflag = 0;
			}
			else if (getflag && (ch != '#')) {
				trans_buffer[nCount++] = '#';
				trans_buffer[nCount++] = ch;
				getflag = 0;
			}
			else if (!getflag && (ch == '#'))
				getflag = 1;
			else
				trans_buffer[nCount++] = ch;
		}
		else if (!getat && puted) {
			if (getflag && (ch == '#'))  //获取到了网页关键字
			{
				act[i] = 0;
				action(act);  //根据网页关键字执行对应程序，输出对应参数
				i = 0;
				puted = 0;
				getflag = 0;
			}
			else if (!getflag && (ch == '#'))
				getflag = 1;
			else
				act[i++] = ch;
		}
		if (nCount >= BUFFER_SIZE) {
			trans_buffer[nCount - 1] = 0;
			fwrite(trans_buffer, sizeof(char), nCount, stdout);
			nCount = 0;
		}
	} while (!feof(fp));
	trans_buffer[nCount] = 0;
	fwrite(trans_buffer, sizeof(char), nCount, stdout);
	fclose(fp);
	return 0;
}

int do_login(char *data)  //登录处理
{
	char password[12];
	int i = 0, j = 0;
	do {
		i++;
	} while (*(data + i) != '=');
	i++;
	while (*(data + i) != '&')  //从post数据中获取用户名和密码
		name[j++] = data[i++];
	name[j] = 0;
	do {
		i++;
	} while (*(data + i) != '=');
	j = 0;
	i++;
	while (*(data + i))
		password[j++] = data[i++];
	password[j] = 0;
	login = 1;

	if (!strcmp(name, "admin") && !strcmp(password, settings.password))  //admin用户
		return ADMIN;
	else if (!strcmp(name, "neutron") && !strcmp(password, "neutron"))  //99view用户（超级用户，用户名和密码固定）
		return SUPER;
	else
		login = 0;
	return NO_USER;  //登录失败
}

char *strlst_findfirst(char *name, char *value)  //根据参数名获取参数值
{
	int i = 0, j = 0, k = 0, hex_num;
	char sname[20], get_name = 0, new_name = 1, hex[3];

	do {
		if (!get_name)  //解析参数名，解析参数值
		{
			if (stdin_data[i] == '=') {
				sname[k] = 0;
				new_name = 0;
				if (!strcmp(sname, name))  //获取到了参数名
					get_name = 1;
				else
					get_name = 0;
				k = 0;
			}
			else if (new_name)
				sname[k++] = stdin_data[i];
			if (i && (stdin_data[i] == '&'))
				new_name = 1;
		}
		else {
			if (stdin_data[i] == '%')  //收到%，收到了汉字或者%
			{
				hex[0] = stdin_data[i + 1];
				hex[1] = stdin_data[i + 2];
				hex[2] = 0;
				sscanf(hex, "%2x", &hex_num);
				value[j++] = (char)hex_num;
				i += 2;
			}
			else if (stdin_data[i] == '&') {
				value[j] = 0;  //获取到了参数值
				return value;
			}
			else
				value[j++] = stdin_data[i];
			if (stdin_data[i + 1] == 0)  //输入数据尾
			{
				value[j] = 0;
				return value;
			}
		}
		i++;
	} while (stdin_data[i]);
	return NULL;
}

void get_ip(unsigned char *ip, char *str)
{
	int tmp[4], i;

	sscanf(str, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
	for (i = 0; i < 4; i++)
		ip[i] = (unsigned char)tmp[i];
}

void paras_set()  //paras.htm提交处理程序
{
	//	unsigned char buffers[100];
	/*
	if (strlst_findfirst("ip1", buffers))
		get_ip(settings.ip1, buffers);
	if (strlst_findfirst("gate1", buffers))
		get_ip(settings.gateway1, buffers);
	if (strlst_findfirst("mask1", buffers))
		get_ip(settings.mask1, buffers);

	if (strlst_findfirst("ip2", buffers))
		get_ip(settings.ip2, buffers);
	if (strlst_findfirst("gate2", buffers))
		get_ip(settings.gateway2, buffers);
	if (strlst_findfirst("mask2", buffers))
		get_ip(settings.mask2, buffers);

	if (strlst_findfirst("debug_ip", buffers))
		get_ip(settings.debug_ip, buffers);
	if (strlst_findfirst("debug_gateway", buffers))
		get_ip(settings.debug_gateway, buffers);
	if (strlst_findfirst("debug_mask", buffers))
		get_ip(settings.debug_mask, buffers);

		if(strlst_findfirst("snmp_ip1",buffers))
			get_ip(settings.snmp_ip1,buffers);
	if (strlst_findfirst("syslog_ip1", buffers))
		get_ip(settings.syslog_ip1, buffers);

		if(strlst_findfirst("snmp_ip2",buffers))
			get_ip(settings.snmp_ip2,buffers);
	if (strlst_findfirst("syslog_ip2", buffers))
		get_ip(settings.syslog_ip2, buffers);
		
	
	if(strlst_findfirst("ip3",buffers))
		get_ip(settings.ip3,buffers);
	if(strlst_findfirst("gate3",buffers))
		get_ip(settings.gateway3,buffers);
	if(strlst_findfirst("mask3",buffers))
		get_ip(settings.mask3,buffers);

	 if(strlst_findfirst("ip4",buffers))
		get_ip(settings.ip4,buffers);
	if(strlst_findfirst("gate4",buffers))
		get_ip(settings.gateway4,buffers);
	if(strlst_findfirst("mask4",buffers))
		get_ip(settings.mask4,buffers);

	if (strlst_findfirst("client", buffers)) {
		if (strlst_findfirst("ntps1", buffers))
			get_ip(settings.ntps1, buffers);
				
		if(strlst_findfirst("ntps2",buffers))
			get_ip(settings.ntps2,buffers);
		if(strlst_findfirst("ntps3",buffers))
			get_ip(settings.ntps3,buffers);

		if(strlst_findfirst("ntps4",buffers))
			get_ip(settings.ntps4,buffers);

		settings.ntp_client = 1;
	}
	else
		settings.ntp_client = 0;
	if (strlst_findfirst("host", buffers))
		settings.host = atoi(buffers);
	if (strlst_findfirst("broadcast", buffers))
		settings.ntp_broadcast = atoi(buffers);
	if (strlst_findfirst("broadaddr", buffers))
		get_ip(settings.Reserve, buffers);
	if (strlst_findfirst("enc", buffers))
		settings.enc = 1;
	else
		settings.enc = 0;

	if (strlst_findfirst("key1", buffers))
		strcpy(settings.key1, buffers);
	if (strlst_findfirst("key2", buffers))
		strcpy(settings.key2, buffers);
		
	if(strlst_findfirst("key3",buffers))       
		strcpy(settings.key3,buffers);
	if(strlst_findfirst("key4",buffers))       
		strcpy(settings.key4,buffers);
	if(strlst_findfirst("key5",buffers))       
		strcpy(settings.key5,buffers);
	if(strlst_findfirst("key6",buffers))       
		strcpy(settings.key6,buffers);
	if(strlst_findfirst("key7",buffers))       
		strcpy(settings.key7,buffers);
	if(strlst_findfirst("key8",buffers))       
		strcpy(settings.key8,buffers);

	if (strlst_findfirst("logtime", buffers))
		settings.logtime = atoi(buffers);
	if (strlst_findfirst("logsize", buffers))
		settings.logsize = atoi(buffers);
		*/
	set_config("/opt/sysfile/syscfg.cfg");
	send_cmd(CMD_CONFIG, NULL);
}

int pass_save()  //保存设置密码
{
	char buffer[20];

	strlst_findfirst((char *)"oldpass", buffer);
	if (strcmp(buffer, settings.password))
		return -1;
	strlst_findfirst((char *)"telpass1", buffer);
	if (strcmp(buffer, ""))
		strcpy(settings.password, buffer);
	else  //密码为空
		memset(settings.password, 0, sizeof(settings.password));
	set_config("/opt/sysfile/syscfg.cfg");
	send_cmd(CMD_CONFIG, NULL);
	return 0;
}

int pass_set()  //密码提交处理
{
	char buffer[20];
	if (strlst_findfirst((char *)"user", buffer)) {
		if (strcmp(buffer, "admin"))
			return -1;
		return pass_save();
	}
	return -1;
}

void get_user(void)  //获取当前用户
{
	// FILE *pt;
	//char a[20];
	char *cookie_user;
	cookie_user = getenv("HTTP_COOKIE");  //读cookie，获取user
										  /*
sscanf(cookie_user,"%s",a);
        printf("%s",a);
pt=fopen("/opt/user.txt","w");
fprintf(pt,"%s",a);
fclose(pt);
*/
	if (cookie_user == NULL) {
		return;
	}
	if (!strncmp(cookie_user, "admin", 5))
		user = ADMIN;
	else if (!strncmp(cookie_user, "neutron", 7))
		user = SUPER;
}

unsigned int GetFileSize(const char *FileName)
{
	struct stat buf;
	if (stat(FileName, &buf))
		return 0;
	return buf.st_size;
}

int put_file_list(char *path, char link)  //打印文件列表，link决定是否是链接
{
	DIR *dp;
	struct dirent *d;
	char string[BUFFER_SIZE], filepath[128];
	int filesize;
	printf("content-type: text/html\n\n");

	if (login)
		printf("Set-Cookie: %s;\n\n", name);  //设置cookie

	sprintf(filepath, "/opt/goahead/web/%s", path);
	if ((dp = opendir(filepath)) == NULL) {
		printf("unable to open directory %s\n", filepath);
		return -1;
	}
	sprintf(string, "<html><meta http-equiv=Content-Type content=\"text/html; charset=gb2312\"><body><center><br><br>"
					"<table COLS=1 WIDTH=\"80%%\"><tr><td BGCOLOR=\"#009ace\"><center><b><font face=\"宋体\"><font size=-1>"
					"<font color=\"#FFFFFF\">文件列表</font></font></font></b></center></td></tr><tr></table>"
					"<table BORDER=0 WIDTH=\"80%%\" BGCOLOR=\"#FFcace\">");
	fwrite(string, strlen(string), 1, stdout);
	while ((d = readdir(dp)) != NULL) {
		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;
		if (!strcmp(d->d_name, "ntpstats"))
			sprintf(string, "<tr><td width=\"10%%\"><font size=-1></font></td><td width=\"70%%\"><font size=-1>"
							"<a href=\"../cgi-bin/mycgi?log/%s\" target=main >%s</a></font></td><td><font size=-1></font></td></tr>",
					d->d_name,
					d->d_name);
		else {
			sprintf(filepath, "/opt/dnts/goahead/web/%s/%s", path, d->d_name);
			filesize = GetFileSize(filepath);
			if (link) {
				if (filesize < 1024)
					sprintf(string, "<tr><td width=\"10%%\"><font size=-1></font></td><td width=\"70%%\"><font size=-1><a href=\"../%s/%s\">%s</a>"
									"</font></td><td><font size=-1>%dB</font></td></tr>",
							path,
							d->d_name,
							d->d_name,
							filesize);
				else
					sprintf(string, "<tr><td width=\"10%%\"><font size=-1></font></td><td width=\"70%%\"><font size=-1><a href=\"../%s/%s\">%s</a>"
									"</font></td><td><font size=-1>%dKB</font></td></tr>",
							path,
							d->d_name,
							d->d_name,
							filesize / 1024);
			}
			else {
				if (filesize < 1024)
					sprintf(string, "<tr><td width=\"10%%\"><font size=-1></font></td><td width=\"70%%\"><font size=-1>%s</font></td>"
									"<td><font size=-1>%dB</font></td></tr>",
							d->d_name,
							filesize);
				else
					sprintf(string, "<tr><td width=\"10%%\"><font size=-1></font></td><td width=\"70%%\"><font size=-1>%s</font></td>"
									"<td><font size=-1>%dKB</font></td></tr>",
							d->d_name,
							filesize / 1024);
			}
		}
		fwrite(string, strlen(string), 1, stdout);
	}
	closedir(dp);
	sprintf(string, "</table><br></center></body></html>");
	fwrite(string, strlen(string), 1, stdout);
	return 0;
}

int main()  //主程序
{
	char *querydata, *datalen;
	int len = 0;

	memset(&settings, 0, sizeof(NTSSetting));
	get_config("/opt/sysfile/syscfg.cfg");  //获取配置文件

	datalen = getenv("CONTENT_LENGTH");
	if (datalen)
		len = atoi(datalen);  //post数据长度
	memset(stdin_data, 0, sizeof(stdin_data));
	fread(stdin_data, 1, len, stdin);
	stdin_data[len] = 0;
	querydata = getenv("QUERY_STRING");  //get数据
	if (querydata == NULL)
		return -1;
	//    FILE *ptr ,*list;
	//   char m[200],n[200];

	get_user();  //获取用户名
	/*
sscanf(querydata,"%s",m);
        printf("%s",m);
ptr=fopen("/opt/test.txt","w");
fprintf(ptr,"%s",m);
fclose(ptr);
*/

	if (!strcmp(querydata, "dologin"))  //登录
	{
		/*
sscanf(querydata,"%s",m);
        printf("%s",m);
ptr=fopen("/opt/test1.txt","w");
fprintf(ptr,"%s",m);
fclose(ptr);
*/

		user = do_login(stdin_data);
		if (user == ADMIN) {
			put_htm_file("alogged.htm");  //admin登录
		}
		else if (user == SUPER)
			put_htm_file("nlogged.htm");  //macrounion登录
		else
			put_htm_file("failed.htm");  //登录失败
	}
	else if (!strcmp(querydata, "dologout"))  //注销
	{
		/*
sscanf(querydata,"%s",m);
        printf("%s",m);
ptr=fopen("/opt/test2.txt","w");
fprintf(ptr,"%s",m);
fclose(ptr);
*/
		login = 1;
		strcpy(name, "nouser");
		put_htm_file("login.htm");
	}
	else if (user != ADMIN && user != SUPER)  //未登录
	{
		put_htm_file("denglu.html");
	}
	/*
	else if (user == ROOT)
	{
		put_sht_file("settings.htm");
	}
	else if (user == NULL_)
	{	
		put_htm_file("success.htm");
	}
*/
	else {
		/*
		sscanf(querydata,"%s",n);
		printf("%s",n);
		list=fopen("/opt/test4.txt","w");
		fprintf(ptr,"%s",n);
		fclose(list);
	*/
		if (!strcmp(querydata, "settings"))  //显示基本信息

			put_sht_file("settings.htm");

		else if (!strcmp(querydata, "paras"))  //显示基本参数
			put_sht_file("paras.htm");
		else if (!strcmp(querydata, "pass"))  //显示密码设置页面
			put_htm_file("apass.htm");
		//	else if (!strcmp(querydata, "log"))  //日志
		//		put_file_list("log", 1);
		//	else if (strstr(querydata, "log"))
		//		put_file_list(querydata, 1);
		else if (!strcmp(querydata, "passa"))  //admin密码和guest密码的设置
		{
			if (pass_set())
				put_htm_file("failed.htm");
			else
				put_htm_file("success.htm");
		}
		else if (!strcmp(querydata, "reset"))  //重启
		{
			put_htm_file("reset.htm");
			restart();
		}
		else if (!strcmp(querydata, "paraset"))  //基本参数设置
		{
			paras_set();
			put_htm_file("success.htm");
		}
		else
			put_sht_file("settings.htm");
	}
	return 0;
}
