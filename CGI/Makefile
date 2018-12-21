EXE   =mycgi
CC    = arm-none-linux-gnueabi-gcc 
OBJS  = main.o
FLAGS = -Wall -O2

$(EXE): $(OBJS)
	$(CC) $^ -o $@
  
main.o: cgi.c  
	$(CC) $(FLAGS) -c $< -o $@
	
clean:
	rm *.o
	rm mycgi
	

