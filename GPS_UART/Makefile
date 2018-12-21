CC=gcc
 
objs=uart.o analyse_gps.o gps_main.o
srcs=uart.c analyse_gps.c gps_main.c
 
gps_test: $(objs)
	$(CC) -o gps_test $(objs)
 
gps_main.o: $(srcs) gps.h
	$(CC) -c  $(srcs) 
 
uart.o:  uart.c
	$(CC) -c  uart.c
 
analyse_gps.o: analyse_gps.c 
	$(CC) -c  analyse_gps.c 
 
clean:  
	rm *.o  
