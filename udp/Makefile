
.PHONY:all
all:client server server_local

client:client.c
	gcc -Wall -o $@ $^
server:server.c
	gcc -Wall -o $@ $^
server_local:server_local.c
	gcc -Wall -o $@ $^
.PHONY:clean
clean:
	rm -f client.o server.o server_local.o server client server_local