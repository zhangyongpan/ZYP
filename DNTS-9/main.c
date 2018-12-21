#include "conf.h"
#include "nmea_fmt_parser.h"

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

NTSSetting global_settings;
pthread_mutex_t mutex_lock;

int get_config(char *filename, NTSSetting *setting)
{
	int fd;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		return -1;
	}
	if (read(fd, setting, sizeof(NTSSetting)) != sizeof(NTSSetting)) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}
void ConfigNetwork(NTSSetting *settings)
{
	char cmd[128];
	memset(cmd, 0, 128);
	sprintf(cmd, "ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d", settings->debug_ip[0], settings->debug_ip[1], settings->debug_ip[2], settings->debug_ip[3], settings->debug_mask[0], settings->debug_mask[1], settings->debug_mask[2], settings->debug_mask[3]);
	system(cmd);

	sprintf(cmd, "route add default gw %d.%d.%d.%d dev eth0", settings->debug_gateway[0], settings->debug_gateway[1], settings->debug_gateway[2], settings->debug_gateway[3]);
	system(cmd);

	memset(cmd, 0, 128);
	sprintf(cmd, "ifconfig eth1 %d.%d.%d.%d netmask %d.%d.%d.%d", settings->ip1[0], settings->ip1[1], settings->ip1[2], settings->ip1[3], settings->mask1[0], settings->mask1[1], settings->mask1[2], settings->mask1[3]);
	system(cmd);
	memset(cmd, 0, 128);
	sprintf(cmd, "route add default gw %d.%d.%d.%d dev eth1", settings->gateway1[0], settings->gateway1[1], settings->gateway1[2], settings->gateway1[3]);

	system(cmd);
}

int read_without_enter(int read_fd, int read_len, char *read_buf)
{
	char loop = 0;
	int len = -1;
	int tmp = 0;
	memset(read_buf, 0, read_len);
	while (1) {
		printf("\nloop == %d\n", loop++);
		len = read(read_fd, read_buf + tmp, read_len - tmp);
		if (len > 0) {
			tmp += len;
		}
		else {
			printf("read error \n");
			return -1;
		}
		//printf("read_buf[%d] %dread_buf[%d] %d\n",  tmp -2, read_buf[tmp -2], tmp -1, read_buf[tmp - 1]);
		if (read_buf[tmp - 1] == 10 && read_buf[tmp - 2] == 13 && tmp > 1)  //�лس�
		{
			printf("read ok \n");
			break;
		}
	}
	return tmp;
}

char *get_ip(char *data)
{
	char tmp[128];
	static char ip[4];
	int loop;
	int i = 0;
	char *ip_t1;
	char *ip_t2;
	memset(ip, 0, 4);
	memset(tmp, 0, 128);
	loop = 0;
	ip_t1 = data;
	ip_t2 = ip_t1;
	while (loop < strlen(data)) {
		if (*ip_t1 == '.') {
			memcpy(tmp, ip_t2, loop);
			loop = 0;
			ip_t2 = ip_t1 + 1;
			//ip_t = data[loop+1];
			printf("tmp = %d   %d\n", atoi(tmp), i);
			ip[i] = atoi(tmp);
			i += 1;
		}
		else if (*ip_t1 == '\0') {
			memcpy(tmp, ip_t2, loop);
			printf("tmp = %d   %d\n", atoi(tmp), i);
			ip[i] = atoi(tmp);
			break;
		}
		ip_t1++;
		loop += 1;
		printf("ip1 = %c   ip2 = %c\n", *ip_t1, *(ip_t2 + 1));
	}
	printf("get ip = %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
	return ip;
}

void set_0(int clientfd)
{
	char tmp[128];
	char read_buf[128];
	char *ip;
	int len = 0;
	int i = 0;
	sprintf(tmp, "\r\nIP Address(NTP): %d.%d.%d.%d \r\nnew IP(enter to skip):", global_settings.ip1[0], global_settings.ip1[1], global_settings.ip1[2], global_settings.ip1[3]);
	write(clientfd, tmp, strlen(tmp));
	memset(read_buf, 0, 128);
	len = read_without_enter(clientfd, 128, read_buf);
	//printf("len = %d %s a\n", len, read_buf);
	if (read_buf[0] == 13 && read_buf[1] == 10) {
	}
	else {
		read_buf[len - 2] = '\0';
		ip = get_ip(read_buf);
		for (i = 0; i < 4; i++) {
			global_settings.ip1[i] = ip[i];
		}
	}
	if (global_settings.gateway1[0]) {
		write(clientfd, "Set GateWay IP Address(NTP): (Y) ?", sizeof("Set GateWay IP Address(NTP): (Y) ?"));
		memset(read_buf, 0, 128);
		len = read_without_enter(clientfd, 128, read_buf);
		if (read_buf[0] == 'y' || read_buf[0] == 'Y') {
			memset(tmp, 0, 128);
			sprintf(tmp, "\r\nGateway IP Address(NTP): %d.%d.%d.%d \r\nnew Gateway IP(enter to skip):", global_settings.gateway1[0], global_settings.gateway1[1], global_settings.gateway1[2], global_settings.gateway1[3]);
			write(clientfd, tmp, strlen(tmp));
			memset(read_buf, 0, 128);
			len = read_without_enter(clientfd, 128, read_buf);
			if (read_buf[0] == 13 && read_buf[1] == 10) {
			}
			else {
				read_buf[len - 2] = '\0';
				ip = get_ip(read_buf);
				for (i = 0; i < 4; i++) {
					global_settings.gateway1[i] = ip[i];
				}
			}
		}
		else if (read_buf[0] == 'n' || read_buf[0] == 'N') {
			memset(global_settings.gateway1, 0, 4);
		}
		else if (read_buf[0] == 13 && read_buf[1] == 10) {
			sprintf(tmp, "\r\nGateway IP Address(NTP) : %d.%d.%d.%d \r\nnew Gateway IP(enter to skip):", global_settings.gateway1[0], global_settings.gateway1[1], global_settings.gateway1[2], global_settings.gateway1[3]);
		}
	}
	else {
		write(clientfd, "Set GateWay IP Address(NTP) : (N) ?", sizeof("Set GateWay IP Address(NTP) : (N) ?"));
		memset(read_buf, 0, 128);
		len = read_without_enter(clientfd, 128, read_buf);
		if (read_buf[0] == 'y' || read_buf[0] == 'Y') {
			memset(tmp, 0, 128);
			sprintf(tmp, "\r\nGateway IP Address(NTP) : %d.%d.%d.%d \r\nnew Gateway IP(enter to skip):", global_settings.gateway1[0], global_settings.gateway1[1], global_settings.gateway1[2], global_settings.gateway1[3]);
			write(clientfd, tmp, strlen(tmp));
			memset(read_buf, 0, 128);
			len = read_without_enter(clientfd, 128, read_buf);
			if (read_buf[0] == 13 && read_buf[1] == 10) {
			}
			else {
				read_buf[len - 2] = '\0';
				ip = get_ip(read_buf);
				for (i = 0; i < 4; i++) {
					global_settings.gateway1[i] = ip[i];
				}
			}
		}
		else if (read_buf[0] == 'n' || read_buf[0] == 'N') {
			memset(global_settings.gateway1, 0, 4);
		}
	}
	sprintf(tmp, "\r\nNetmask : %d.%d.%d.%d \r\nnew Netmask(enter to skip):", global_settings.mask1[0], global_settings.mask1[1], global_settings.mask1[2], global_settings.mask1[3]);
	write(clientfd, tmp, strlen(tmp));
	memset(read_buf, 0, 128);
	len = read_without_enter(clientfd, 128, read_buf);
	if (read_buf[0] == 13 && read_buf[1] == 10) {
	}
	else {
		read_buf[len - 2] = '\0';
		ip = get_ip(read_buf);
		for (i = 0; i < 4; i++) {
			global_settings.mask1[i] = ip[i];
		}
	}

	sprintf(tmp, "\r\nNetwork management IP_Address : %d.%d.%d.%d \r\nnew IP(enter to skip):", global_settings.debug_ip[0], global_settings.debug_ip[1], global_settings.debug_ip[2], global_settings.debug_ip[3]);
	write(clientfd, tmp, strlen(tmp));
	memset(read_buf, 0, 128);
	len = read_without_enter(clientfd, 128, read_buf);
	//printf("len = %d %s a\n", len, read_buf);
	if (read_buf[0] == 13 && read_buf[1] == 10) {
	}
	else {
		read_buf[len - 2] = '\0';
		ip = get_ip(read_buf);
		for (i = 0; i < 4; i++) {
			global_settings.debug_ip[i] = ip[i];
		}
	}
	if (global_settings.debug_gateway[0]) {
		write(clientfd, "Set GateWay Network management IP_Address : (Y) ?", sizeof("Set GateWay Network management IP_Address : (Y) ?"));
		memset(read_buf, 0, 128);
		len = read_without_enter(clientfd, 128, read_buf);
		if (read_buf[0] == 'y' || read_buf[0] == 'Y') {
			memset(tmp, 0, 128);
			sprintf(tmp, "\r\nGateway Network management IP_Address : %d.%d.%d.%d \r\nnew Gateway IP(enter to skip):", global_settings.debug_gateway[0], global_settings.debug_gateway[1], global_settings.debug_gateway[2], global_settings.debug_gateway[3]);
			write(clientfd, tmp, strlen(tmp));
			memset(read_buf, 0, 128);
			len = read_without_enter(clientfd, 128, read_buf);
			if (read_buf[0] == 13 && read_buf[1] == 10) {
			}
			else {
				read_buf[len - 2] = '\0';
				ip = get_ip(read_buf);
				for (i = 0; i < 4; i++) {
					global_settings.debug_gateway[i] = ip[i];
				}
			}
		}
		else if (read_buf[0] == 'n' || read_buf[0] == 'N') {
			memset(global_settings.debug_gateway, 0, 4);
		}
		else if (read_buf[0] == 13 && read_buf[1] == 10) {
			sprintf(tmp, "\r\nGateway Network management IP_Address : %d.%d.%d.%d \r\nnew Gateway IP(enter to skip):", global_settings.debug_gateway[0], global_settings.debug_gateway[1], global_settings.debug_gateway[2], global_settings.debug_gateway[3]);
		}
	}
	else {
		write(clientfd, "Set GateWay Network management IP_Address : (N) ?", sizeof("Set GateWay Network management IP_Address : (N) ?"));
		memset(read_buf, 0, 128);
		len = read_without_enter(clientfd, 128, read_buf);
		if (read_buf[0] == 'y' || read_buf[0] == 'Y') {
			memset(tmp, 0, 128);
			sprintf(tmp, "\r\nGateway Network management IP_Address : %d.%d.%d.%d \r\nnew Gateway IP(enter to skip):", global_settings.debug_gateway[0], global_settings.debug_gateway[1], global_settings.debug_gateway[2], global_settings.debug_gateway[3]);
			write(clientfd, tmp, strlen(tmp));
			memset(read_buf, 0, 128);
			len = read_without_enter(clientfd, 128, read_buf);
			if (read_buf[0] == 13 && read_buf[1] == 10) {
			}
			else {
				read_buf[len - 2] = '\0';
				ip = get_ip(read_buf);
				for (i = 0; i < 4; i++) {
					global_settings.debug_gateway[i] = ip[i];
				}
			}
		}
		else if (read_buf[0] == 'n' || read_buf[0] == 'N') {
			memset(global_settings.debug_gateway, 0, 4);
		}
	}
	sprintf(tmp, "\r\nNetwork management_Netmask : %d.%d.%d.%d \r\nnew Network management_Netmask(enter to skip):", global_settings.debug_mask[0], global_settings.debug_mask[1], global_settings.debug_mask[2], global_settings.debug_mask[3]);
	write(clientfd, tmp, strlen(tmp));
	memset(read_buf, 0, 128);
	len = read_without_enter(clientfd, 128, read_buf);
	if (read_buf[0] == 13 && read_buf[1] == 10) {
	}
	else {
		read_buf[len - 2] = '\0';
		ip = get_ip(read_buf);
		for (i = 0; i < 4; i++) {
			global_settings.debug_mask[i] = ip[i];
		}
	}

	//	printf("gateway ip = %d.%d.%d.%d", tmp_setting.debug_mask[0],tmp_setting.debug_mask[1],tmp_setting.debug_mask[2],tmp_setting.debug_mask[3]);
}
int set_config(char *filename, NTSSetting *settings)
{
	int fd;

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC)) < 0) {
		return -1;
	}
	if (write(fd, settings, sizeof(NTSSetting)) != sizeof(NTSSetting)) {
		close(fd);
		printf("set_config:Write %s failed!\n", filename);
		return -1;
	}

	sync();
	close(fd);
	return 0;
}

void change_config(int clientfd)
{
	char cmd[8];
	int len = 0;
show_:
	write(clientfd, (const unsigned char *)"\r\n\r\nChange Setup:\r\n", strlen("\r\n\r\nChange Setup:\r\n"));
	write(clientfd, (const unsigned char *)"0 Server configuration:\r\n", strlen("0 Server configuration:\r\n"));
	write(clientfd, (const unsigned char *)"1 Reboot:\r\n", strlen("1 Reboot:\r\n"));
	write(clientfd, (const unsigned char *)"2 Exit without save:\r\n", strlen("2 Exit without save:\r\n"));
	write(clientfd, (const unsigned char *)"3 Save and exit:                Your choice ? ", strlen("3 Save and exit:                Your choice ? "));
	len = read_without_enter(clientfd, 8, cmd);
	if (len < 2) {
		return;
	}
	if (cmd[1] == 13 && cmd[2] == 10) {
		switch (cmd[0]) {
		case '0':
			set_0(clientfd);
			break;
		case '1':
			system("reboot");
			break;
		case '2':
			return;
			break;
		case '3':
			pthread_mutex_lock(&mutex_lock);
			if (!set_config("/opt/sysfile/syscfg.cfg", (NTSSetting *)&global_settings)) {
				write(clientfd, "save config infomation successed!", strlen("save config infomation successed!"));
				system("reboot");
			}
			else {
				write(clientfd, "Save config infomation error! Do it again.", strlen("Save config infomation error! Do it again."));
			}
			pthread_mutex_unlock(&mutex_lock);
			break;
		default:
			goto show_;
			break;
		}
	}
	else {
		goto show_;
	}
	goto show_;
}
void *run(void *d)
{
	int client_fd = *(int *)d;
	char temp[1024];
	char read_buf[1024];
	memset(read_buf, 0, 1024);
	memset(temp, 0, 1024);
	write(client_fd, "\n\r*** DNTS-9 NTP Time Server ***", sizeof("\n\r*** DNTS-9 NTP Time Server ***"));
	write(client_fd, "\n\r*** Tech Support:010-62130078 ***", sizeof("\n\r*** Tech Support:010-62130078 ***"));
	sprintf(temp, "\n\r*** Device sn: %s ***", global_settings.Device_sn);
	write(client_fd, temp, strlen(temp));
	memset(temp, 0, 1024);
	sprintf(temp, "\n\rSoftware version %s", global_settings.version);
	write(client_fd, temp, strlen(temp));
	write(client_fd, "\n\rPress Enter for Setup Mode\n\r", strlen("\n\rPress Enter for Setup Mode\n\r"));
	fd_set readset;
	int noto;
	struct timeval telnet_tv;
	telnet_tv.tv_sec = 5;
	telnet_tv.tv_usec = 0;
	FD_ZERO(&readset);
	FD_SET(client_fd, &readset);
	noto = select(client_fd + 1, &readset, 0, 0, &telnet_tv);
	if (!noto) {
		write(client_fd, "too long time no enter\n\r", strlen("too long time no enter\n\r"));
		close(client_fd);
	}
	if (FD_ISSET(client_fd, &readset)) {
		write(client_fd, "\n\r*** basic parameters ***", strlen("\n\r*** basic parameters ***"));
		sprintf(temp, "\n\rIP: %d.%d.%d.%d,", global_settings.ip1[0], global_settings.ip1[1], global_settings.ip1[2], global_settings.ip1[3]);
		write(client_fd, temp, strlen(temp));
		if (global_settings.gateway1[0]) {
			memset(temp, 0, 1024);
			sprintf(temp, "gateway: %d.%d.%d.%d,", global_settings.gateway1[0], global_settings.gateway1[1], global_settings.gateway1[2], global_settings.gateway1[3]);
			write(client_fd, temp, strlen(temp));
		}
		else {
			write(client_fd, "no gateway set,", strlen("no gateway set,"));
		}
		memset(temp, 0, 1024);
		sprintf(temp, "netmask: %d.%d.%d.%d", global_settings.mask1[0], global_settings.mask1[1], global_settings.mask1[2], global_settings.mask1[3]);
		write(client_fd, temp, strlen(temp));
		memset(temp, 0, 1024);
		write(client_fd, "\n\rDNTS Type Name: DNTS-9", strlen("\n\rDNTS Type Name: DNTS-9"));
		write(client_fd, "\n\rLogin:", strlen("\n\rLogin:"));

		change_config(client_fd);
		printf("config finished\n");
		close(client_fd);
	}
	return ((void *)0);
}

void *telnet_config()
{
	int fd = 0, clientfd = 0, ret = 0;
	fd_set readset;
	int noto;
	struct timeval telnet_tv;
	ret = (int)signal(SIGPIPE, SIG_IGN);
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		exit(-1);
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if ((bind(fd, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in))) == -1) {
		printf("bind error!\n");
		exit(-1);
	}
	if (listen(fd, 1) == -1) {
		printf("listen error\n");
		exit(-1);
	}
	//	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(struct sockaddr_in);
	while (1) {
		telnet_tv.tv_sec = 5;
		telnet_tv.tv_usec = 0;
		FD_ZERO(&readset);
		FD_SET(fd, &readset);
		noto = select(fd + 1, &readset, 0, 0, &telnet_tv);
		if (!noto) {
			usleep(300 * 1000);
			continue;
		}

		if (FD_ISSET(fd, &readset)) {
			if ((clientfd = accept(fd, (struct sockaddr *)(&clientfd), &len)) == -1) {
				printf("accept error\n");
				exit(-1);
			}
			//		printf("client IP is %s\n",inet_ntoa(clientaddr.sin_addr));
			//      printf("client Port is %d\n", ntohs(clientaddr.sin_port));

			pthread_t tid;
			pthread_create(&tid, 0, run, &clientfd);
		}
		else {
			usleep(300 * 1000);
		}
	}
	close(fd);
	return ((void *)0);
}
#if 0
static int gpio_read(int pin)
{
	char path[64];
	char value_str[3];
	int fd;
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("failed open %d gpio\n", pin);
		return -1;
	}
	if (read(fd, value_str, 3) < 0) {
		printf("failed to read value\n");
		return -1;
	}
	close(fd);
	return (atoi(value_str));
}

static unsigned char net_num_get()
{
	char io1 = 0, io2 = 0, io3 = 0, io4 = 0;
	io1 = gpio_read(41) == 1 ? 0 : 1;
	io2 = gpio_read(40) == 1 ? 0 : 1;
	io3 = gpio_read(13) == 1 ? 0 : 1;
	io4 = gpio_read(12) == 1 ? 0 : 1;
	return io1 * 8 + io2 * 4 + io1 * 2 + io4;
}
#endif
static int gpio_export(int pin)
{
	char buffer[64];
	int len;
	int fd;
	fd = open("/sys/class/gpio/export", O_RDONLY);
	if (fd < 0) {
		printf("failed to open export for writing\n");
		return -1;
	}
	len = snprintf(buffer, sizeof(buffer), "%d", pin);
	if (write(fd, buffer, len) < 0) {
		printf("failed to export gpio %d\n", pin);
		return -1;
	}
	close(fd);
	return 0;
}

static int gpio_unexport(int pin)
{
	char buffer[64];
	int len;
	int fd;
	fd = open("/sys/class/gpio/unexport", O_RDONLY);
	if (fd < 0) {
		printf("failed to open unexport for writing\n");
		return -1;
	}
	len = snprintf(buffer, sizeof(buffer), "%d", pin);
	if (write(fd, buffer, len) < 0) {
		printf("failed to unexport gpio %d\n", pin);
		return -1;
	}
	close(fd);
	return 0;
}

static int gpio_direction(int pin, int dir)
{
	//dir: 0-->IN 1-->OUT
	static const char dir_str[] = "in\0out";
	char path[64];
	int fd;
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) {
		printf("failed to open gpio direction for writing\n");
		return -1;
	}
	if (write(fd, &dir_str[dir == 0 ? 0 : 3], dir == 0 ? 2 : 3) < 0) {
		printf("failed to set direction\n");
		return -1;
	}
	close(fd);
	return 0;
}
void gpio_init(void)
{
	gpio_unexport(12);
	gpio_unexport(13);
	gpio_unexport(40);
	gpio_unexport(41);
	gpio_export(12);
	usleep(1000 * 10);
	gpio_export(13);
	usleep(1000 * 10);
	gpio_export(40);
	usleep(1000 * 10);
	gpio_export(41);
	usleep(1000 * 10);
	gpio_direction(12, 0);
	usleep(1000 * 10);
	gpio_direction(13, 0);
	usleep(1000 * 10);
	gpio_direction(40, 0);
	usleep(1000 * 10);
	gpio_direction(41, 0);
	usleep(1000 * 10);
}

void m_sethwf(int fd, int on)
{
	struct termios tty;

	tcgetattr(fd, &tty);
	if (on)
		tty.c_cflag |= CRTSCTS;
	else
		tty.c_cflag &= ~CRTSCTS;
	tcsetattr(fd, TCSANOW, &tty);
}

/* Set RTS line. Sometimes dropped. Linux specific? */
static void m_setrts(int fd)
{
	int mcs = 0;

	ioctl(fd, TIOCMGET, &mcs);
	mcs |= TIOCM_RTS;
	ioctl(fd, TIOCMGET, &mcs);
}
void m_setparms(int fd)
{
	int spd = -1;

	struct termios tty;

	tcgetattr(fd, &tty);
	spd = B115200;

	if (spd != -1) {
		cfsetospeed(&tty, (speed_t)spd);
		cfsetispeed(&tty, (speed_t)spd);
	}

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	/* Set into raw, no echo mode&& abs(now_serial_time.tv_sec - last_serial_time.tv_sec) < 60 */
	tty.c_iflag = IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cflag |= CLOCAL | CREAD;

	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	tty.c_cflag &= ~(PARENB | PARODD);

	tty.c_cflag &= ~CSTOPB;

	tcsetattr(fd, TCSANOW, &tty);

	m_setrts(fd);
	m_sethwf(fd, 0);
}

int ip_serial(void)
{
	int fd, n = 0;
	fd = open(IPDEVICE, O_RDWR | O_NDELAY | O_NOCTTY);
	if (fd < 0) {
		printf("open serial port failed\n");
		return -1;
	}
	n = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, n & ~O_NDELAY);
	m_setparms(fd);

	printf("open serial port successful\n");
	return fd;
}
int stm32_serial(void)
{
	int fd, n = 0;
	fd = open(SERIALDEVICE, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		printf("Open serial port failed!\n");
		return -1;
	}
	n = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, n & ~O_NDELAY);
	m_setparms(fd);

	printf("Open serial port successfully!\n");
	return fd;
}

void get_eth1(char *buffer)
{
	int sockfd;
	struct ifreq ifr;
	struct sockaddr_in sin;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		exit(-1);
	}
	strcpy(ifr.ifr_name, "eth1");
	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0) {
		perror("ioctl error");
		exit(-1);
	}
	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	strcpy(buffer, inet_ntoa(sin.sin_addr));
	close(sockfd);
}

int NMEA_0183(char *messag, int num_port)
{
	char buf[16];
	int ip[4];
	int err;
	uint16_t cksum;
	get_eth1(buf);
	sscanf(buf, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	sprintf(messag, "$IPADDR,0,%d,%d,%d,%d,%d*00", num_port, ip[0], ip[1], ip[2], ip[3]);

	if ((err = CalcMsgSum_NMEA(messag, strlen(messag), &cksum)) < 0) {
		perror("calc cksum error");
		return -1;
	}
	if ((err = SetMsgSum_NMEA(messag, strlen(messag), cksum)) < 0) {
		perror("set cksum error");
		return -2;
	}

	return 0;
}
#if 0
void read_0183(char *dst, char *src)
{
	assert((dst != NULL) && (src != NULL));
	while (*src != '\0') {
		*dst++ = *src++;
	}
}
#endif
static int read_fd(int fd, char *buffer)
{
	unsigned char ch;
	int err, length = 0;
	char *buffer_nmea = buffer;

	enum STA { RX_Idle,
			   RX_RECV,
			   RX_END,
			   RX_FIN };
	static enum STA rxstate = RX_Idle;

	while ((err = read(fd, &ch, 1)) > 0) {
		//	printf("ch:%c\n", ch);
		switch (rxstate) {
		case RX_Idle:
			length = 0;
			if (ch == '$') {
				buffer_nmea[length] = ch;
				length++;
				rxstate = RX_RECV;
			}
			else {
				rxstate = RX_Idle;
			}
			break;
		case RX_RECV:
			if (ch == '\r') {
				buffer_nmea[length] = ch;
				length++;
				rxstate = RX_END;
			}
			else {
				buffer_nmea[length] = ch;
				length++;
				rxstate = RX_RECV;
			}
			break;
		case RX_END:
			if (ch == '\n') {
				buffer_nmea[length] = ch;
				buffer_nmea[length + 1] = '\0';
				rxstate = RX_Idle;
				return 1;
			}
			else {
				memset(buffer_nmea, 0, 128);
				length = 0;
				rxstate = RX_Idle;
				return -1;
			}
		default:
			length = 0;
			rxstate = RX_Idle;
			break;
		}
	}
	return -2;
}
int read_num(char *filname)
{
	char num;
	int fd;
	fd = open(filname, O_RDONLY);
	if (fd < 0) {
		perror("open error");
	}
	if (read(fd, &num, sizeof(char)) < 0) {
		perror("read error");
	}
	return atoi(&num);
}

void *nmea_ip()
{
	int ip_fd, stm32_fd;
	pid_t pid;
	char buffer[128];

	//	gpio_init();  //初始化gpio
	int num_net = read_num(NUM_FILE);
	printf("num_net:%d\n", num_net);

	if ((ip_fd = ip_serial()) < 0) {
		printf("open ip serail failed\n");
		exit(-1);
	}

	if ((stm32_fd = stm32_serial()) < 0) {
		printf("open stm32 serial failed\n");
		exit(-1);
	}

	pid = fork();

	if (pid < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0) {
		if (num_net == 1) {
			char rev_flag;
			char buf[128];
			char message1[128], message2[128], message3[128], message4[128];

			fd_set uread;
			int len;
			char tab[] = "\r\n";
			int time_out, c = 0;
			int flag1 = 0;
			char timeout[30];

			struct timeval ut1;
			ut1.tv_sec = 2;
			ut1.tv_usec = 0;

			while (1) {
				if (NMEA_0183(buf, num_net) < 0)  //num_net :1 的0183报文生成
				{
					perror("NMEA_0183 error");
				}
				strcpy(message1, buf);

				FD_ZERO(&uread);
				FD_SET(ip_fd, &uread);

				time_out = select(ip_fd + 1, &uread, 0, 0, &ut1);
				printf("time_out:%d\n", time_out);

				if (c == 0) {
					flag1 = 2;
					write(ip_fd, &flag1, sizeof(int));
					c = 1;
				}
				else {
					if (flag1 == 4) {
						flag1 = 1;
					}
					if (time_out > 0) {
						flag1++;
						printf("flag1_1:%d\n", flag1);
						write(ip_fd, &flag1, sizeof(int));
					}

					if (time_out == 0) {
						flag1++;
						printf("flag1_2:%d\n", flag1);
						write(ip_fd, &flag1, sizeof(int));
					}
				}
				printf("flag1:%d\n", flag1);

				if (FD_ISSET(ip_fd, &uread)) {
					len = read_fd(ip_fd, buffer);

					rev_flag = *(buffer + 10);
					printf("rcv_flag:%c\n", rev_flag);
					if (rev_flag == '2') {
						strcpy(message2, buffer);
						printf("message2:%s\n", message2);
					}
					else if (rev_flag == '3') {
						strcpy(message3, buffer);
						printf("message3:%s\n", message3);
					}
					else if (rev_flag == '4') {
						strcpy(message4, buffer);
						printf("message4:%s\n", message4);
					}
				}
				if (time_out == 0) {
					printf("switch flag1:%d", flag1);
					switch (flag1) {
					case 2:
						sprintf(timeout, "$IPADDR,0,%d,0,0,0,0*00\r\n", (flag1 + 2));
						strcpy(message4, timeout);
						break;
					case 3:
						sprintf(timeout, "$IPADDR,0,%d,0,0,0,0*00\r\n", (flag1 - 1));
						strcpy(message2, timeout);
						break;
					case 4:
						sprintf(timeout, "$IPADDR,0,%d,0,0,0,0*00\r\n", (flag1 - 1));
						strcpy(message3, timeout);
						break;
					default:
						break;
					}
				}
				write(stm32_fd, message1, strlen(message1));
				write(stm32_fd, tab, strlen(tab));
				write(stm32_fd, message2, strlen(message2));
				write(stm32_fd, message3, strlen(message3));
				write(stm32_fd, message4, strlen(message4));

				printf("\n1:%s\n 2:%s\n 3:%s\n 4:%s\n", message1, message2, message3, message4);
				sleep(1);
			}
		}
		else {
			fd_set uread;
			struct timeval ut2;
			int len;
			ut2.tv_sec = 2;
			ut2.tv_usec = 0;
			int flag;
			char tb[] = "\r\n";

			while (1) {
				FD_ZERO(&uread);
				FD_SET(ip_fd, &uread);
				select(ip_fd + 1, &uread, 0, 0, &ut2);
				if (FD_ISSET(ip_fd, &uread)) {
					switch (num_net) {
					case 2:
						if (NMEA_0183(buffer, num_net) < 0)  //num_net :2 的0183报文生成
						{
							perror("NMEA_0183 error");
						}

						if ((len = read(ip_fd, &flag, sizeof(int))) < 0) {
							perror("read error");
						}
						printf("flag2:%d\n", flag);
						if (flag == 2) {
							write(ip_fd, buffer, strlen(buffer));
							write(ip_fd, tb, strlen(tb));
						}

						break;
					case 3:
						if (NMEA_0183(buffer, num_net) < 0)  //num_net :3 的0183报文生成
						{
							perror("NMEA_0183 error");
						}
						if ((len = read(ip_fd, &flag, sizeof(int))) < 0) {
							perror("read error");
						}
						printf("flag3:%d\n", flag);
						if (flag == 3) {
							write(ip_fd, buffer, strlen(buffer));
							write(ip_fd, tb, strlen(tb));
						}

						break;
					case 4:
						if (NMEA_0183(buffer, num_net) < 0)  //num_net :4 的0183报文生成
						{
							perror("NMEA_0183 error");
						}
						if ((len = read(ip_fd, &flag, sizeof(int))) < 0) {
							perror("read error");
						}
						printf("flag4:%d\n", flag);

						if (flag == 4) {
							write(ip_fd, buffer, strlen(buffer));
							write(ip_fd, tb, strlen(tb));
						}
						break;
					default:
						break;
					}
				}
				usleep(10);
			}
		}
	}
	else {
		struct timeval ut3;
		ut3.tv_sec = 2;
		ut3.tv_usec = 0;
		fd_set stmread;
		int ret, len;
		char stmbuf[64];
		char *p = NULL;
		char num[10] = {0};
		static int write_flag;

		while (1) {
			FD_ZERO(&stmread);
			FD_SET(stm32_fd, &stmread);
			ret = select(stm32_fd + 1, &stmread, 0, 0, &ut3);
			memset(num, '\0', 10);
			memset(stmbuf, 0, 64);
			if (FD_ISSET(stm32_fd, &stmread)) {
				len = read_fd(stm32_fd, stmbuf);
				printf("stmbuf:%s len:%d\n", stmbuf, len);
				if (len == 1) {
					if (strstr(stmbuf, "$SATNUM") != NULL) {
						write_flag = 0;
						p = strchr(stmbuf, ',');
						if (atoi((p + 1)) == 0) {
							sscanf(stmbuf, "%*[^,],%*[^,],%[^*]", num);
							global_settings.sat_num[0] = atoi(num);
							printf("GPS num:%d\n", global_settings.sat_num[0]);
						}
						else if (atoi((p + 1)) == 1) {
							sscanf(stmbuf, "%*[^,],%*[^,],%[^*]", num);
							global_settings.sat_num[1] = atoi(num);
							printf("beidu num:%d\n", global_settings.sat_num[1]);
						}
						else if (atoi((p + 1)) == 2) {
							sscanf(stmbuf, "%*[^,],%*[^,],%[^*]", num);
							global_settings.sat_num[2] = atoi(num);
							printf("Glonass num:%d\n", global_settings.sat_num[2]);
						}
						else {
							printf(" type error\n");
						}
					}
					else if (strstr(stmbuf, "$ANTSTATE") != NULL) {
						p = strchr(stmbuf, ',');
						if (atoi((p + 1)) == 1) {
							sscanf(stmbuf, "%*[^,],%*[^,],%[^*]", num);
							strcpy(global_settings.tx_sta1, num);
							printf("tx_sta1:%s\n", global_settings.tx_sta1);
						}
						else if (atoi((p + 1)) == 2) {
							sscanf(stmbuf, "%*[^,],%*[^,],%[^*]", num);
							strcpy(global_settings.tx_sta2, num);
							printf("tx_sta2:%s\n", global_settings.tx_sta2);
						}
						else {
							printf("error\n");
						}
					}
					else if (strstr(stmbuf, "$OSCSTATE") != NULL) {
						write_flag = 1;
						sscanf(stmbuf, "%*[^,],%*[^,],%[^*]", num);
						strcpy(global_settings.osstate, num);
						printf("osstate:%s\n", global_settings.osstate);
					}
				}
			}
			pthread_mutex_lock(&mutex_lock);
			if (write_flag == 1) {
				if (!set_config(CONFIG_FILE, (NTSSetting *)&global_settings)) {
					printf("sysfile write successful\n");
				}
			}
			pthread_mutex_unlock(&mutex_lock);
			usleep(200000);
		}
	}

	close(ip_fd);
	close(stm32_fd);
	return ((void *)0);
}

int main(void)
{
	int tid_telnet, tid_ip;
	struct msg_info some_data;
	int msgid;
	char buf[] = "hello world";
	pthread_mutex_init(&mutex_lock, NULL);

	if (get_config(CONFIG_FILE, &global_settings)) {
		printf("read file error\n");
		return -1;
	}

	memcpy(global_settings.version, "V1.0", 4);
	memcpy(global_settings.Device_sn, "DNTS-94", 7);

	ConfigNetwork(&global_settings);
	pthread_create((pthread_t *)&tid_telnet, NULL, telnet_config, NULL);
	pthread_create((pthread_t *)&tid_ip, NULL, nmea_ip, NULL);

	if ((msgid = msgget((key_t)12345, 0666 | IPC_CREAT)) == -1) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	some_data.mytype = 1;
	some_data.i = 0;
	memcpy(some_data.s, buf, strlen(buf));

	while (1) {
		if ((msgsnd(msgid, (void *)&some_data, sizeof(some_data.i) + sizeof(some_data.s), 0)) == -1) {
			perror("msgsnd");
			exit(EXIT_FAILURE);
		}
		some_data.i++;
		sleep(1);
	}

	return 0;
}
