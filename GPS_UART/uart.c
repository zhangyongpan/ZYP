#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newttys1, oldttys1;
	if (tcgetattr(fd, &oldttys1) != 0) {
		perror("set up serial 1");
		return -1;
	}

	bzero(&newttys1, sizeof(newttys1));
	newttys1.c_cflag |= (CLOCAL | CREAD);

	newttys1.c_cflag &= ~CSIZE;

	switch (nBits)  //数据位
	{
	case 7:
		newttys1.c_cflag |= CS7;
		break;
	case 8:
		newttys1.c_cflag |= CS8;
		break;
	}

	switch (nEvent)  //设置校验位
	{
	case '0':
		newttys1.c_cflag |= PARENB;
		newttys1.c_iflag |= (INPCK | ISTRIP);
		newttys1.c_cflag |= PARODD;
		break;
	case 'E':
		newttys1.c_cflag |= PARENB;
		newttys1.c_iflag |= (INPCK | ISTRIP);
		newttys1.c_cflag &= ~PARODD;
		break;
	case 'N':
		newttys1.c_cflag &= ~PARENB;
		break;
	}
	switch (nSpeed) {
	case 2400:
		cfsetispeed(&newttys1, B2400);
		cfsetispeed(&newttys1, B2400);
		break;

	case 4800:
		cfsetispeed(&newttys1, B4800);
		cfsetospeed(&newttys1, B4800);
		break;
	case 9600:
		cfsetispeed(&newttys1, B9600);
		cfsetospeed(&newttys1, B9600);
		break;
	case 115200:
		cfsetispeed(&newttys1, B115200);
		cfsetospeed(&newttys1, B115200);
		break;
	default:
		cfsetispeed(&newttys1, B9600);
		cfsetospeed(&newttys1, B9600);
		break;
	}
	if (nStop == 1) {
		newttys1.c_cflag &= ~CSTOPB;
	}
	else if (nStop == 2) {
		newttys1.c_cflag |= CSTOPB;
	}

	newttys1.c_cc[VTIME] = 0;
	newttys1.c_cc[VMIN] = 0;

	tcflush(fd, TCIFLUSH);

	if ((tcsetattr(fd, TCSANOW, &newttys1)) != 0) {
		perror("come set error");
		return -1;
	}
	return 0;
}
