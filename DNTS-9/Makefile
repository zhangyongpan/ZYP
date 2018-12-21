EXE   = main
CC    = arm-none-linux-gnueabi-gcc # -m32
OBJS  = main.o nmea_fmt_parser.o
FLAGS = -Wall -O2
$(EXE): $(OBJS)
	$(CC) $^ -o $@  -lpthread 

main.o: main.c
	$(CC) $(FLAGS) -c $< -o $@ -lpthread 
nmea_fmt_parser.o: nmea_fmt_parser.c
	$(CC) $(FLAGS) -c $< -o $@  -std=gnu99

clean:
	rm -f *.o
	rm -f main
	
	
