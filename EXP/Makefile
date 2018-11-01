objects =  receive.o

helloworld: $(objects)
	gcc -g -Wall -o receive $(objects)  -lpthread -L/usr/lib/mysql -lmysqlclient
receive.o: receive.c hash_lin.h
	gcc -g -Wall -c receive.c  -lpthread 
clean:
	rm -f *.o receive