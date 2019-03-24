host1: socket1.c
	gcc -o socket1 socket1.c -I.

host2: socket2.c
	gcc -o socket2 socket2.c -I.

bbg1: socket1.c
	arm-linux-gcc -o socket1 socket1.c -lpthread -lrt -D_BSD_SOURCE -I.

bbg2: socket2.c
	arm-linux-gcc -o socket2 socket2.c -lpthread -lrt -D_BSD_SOURCE -I.

main: main.c
	arm-linux-gcc -o i2c_sensors main.c -lpthread -lrt -D_BSD_SOURCE -lm -I.

bbg4: LightSensor.c
	arm-linux-gcc -o LightSensor LightSensor.c -lpthread -lrt -D_BSD_SOURCE -lm -I.

bbg5: TempSensor.c
	arm-linux-gcc -o TempSensor TempSensor.c -lpthread -lrt -D_BSD_SOURCE -lm -I.
