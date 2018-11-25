all : ser

ser: ser.o thread.o
	gcc -o ser ser.o thread.o -lpthread

ser.o: ser.c
	gcc -c ser.c -g

thread.o : thread.c
	gcc -c thread.c -g

clean:
	rm -rf *.o ser


