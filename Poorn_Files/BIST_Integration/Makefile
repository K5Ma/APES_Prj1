main: main.c
	arm-linux-gcc -o integration main.c -lpthread -lrt -D_BSD_SOURCE -lm -I.

client: socket_client.c
	gcc -o socket_client socket_client.c -I.

