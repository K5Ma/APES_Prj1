host: socket_client.c
	gcc -o host_socket_client socket_client.c -I.

bbg: socket_server.c
	arm-linux-gcc -o bbg_socket_server socket_server.c -lpthread -lrt -D_BSD_SOURCE -I.
