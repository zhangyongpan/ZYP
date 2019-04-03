#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <asm/termios.h>
#include <strings.h>
#include "serial.h"

#define ttyS0  0
#define ttyS1  1
#define ttyS2  2
#define ttyS3  3
#define ttyS4  4
#define ttyS5  5
#define ttyS6  6
#define ttyS7  7

static int n_com_port = 3;

int main (int argc, char *argv[])
{
	int ret = -1;
	int len_tty = -1;
	int fd_rs485;
	unsigned char buf_tty[256];


	if (argc > 1) {
		n_com_port = strtol( argv[1], NULL, 10 );
	}

	ret = OpenComPort(n_com_port, 115200, 8, "1", 'N');
	if (ret < 0) {
		fprintf(stderr, "Error: Opening Com Port %d\n", n_com_port);
		return ret;
	}else{
		printf("Open Com Port %d Success, Now going to read port\n", n_com_port);
	}

	while(1){
		bzero(buf_tty, sizeof(buf_tty));
#if 1
        len_tty = ReadComPort(buf_tty, 255);

		if (len_tty < 0) {
			printf("Error: Read Com Port\n");
			break;
		}

		if (len_tty == 0) {
			write(STDOUT_FILENO, ".", sizeof("."));
			continue;
		}
#endif
		printf("Recv: %d bytes, [%s]\n", len_tty, buf_tty);
		
		len_tty = WriteComPort(buf_tty, len_tty);
		WriteComPort(" recved:", sizeof(" recved:"));
		if (len_tty < 0) {
			printf("Error: WriteComPort Error\n");
		}
		// delay 500 ms to let data transfer complete
		// here will cause bugs 
		usleep(500 * 1000);
	}

	CloseComPort();

	printf("Program Terminated\n");

	return(0);
}

