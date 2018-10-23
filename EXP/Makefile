objects =  6666.o

helloworld: $(objects)
	gcc -g -Wall -o 6666 $(objects)  -lpthread -L/usr/lib/mysql -lmysqlclient
6666.o: 6666.c hash_lin.h
	gcc -g -Wall -c 6666.c  -lpthread 
clean:
	rm -f *.o 6666