EXEC	= uart
OBJS    = test_uart.o serial.o

//CROSS = arm-none-linux-gnueabi-
//CC	= $(CROSS)gcc 
CC =gcc
STRIP	= $(CROSS)strip
CFLAGS	= -g -O2

all:  clean $(EXEC)

$(EXEC):$(OBJS)
	$(CC) $(CFLAGS)	-o $@ $(OBJS)

clean:
	-rm -f $(EXEC) *.o

