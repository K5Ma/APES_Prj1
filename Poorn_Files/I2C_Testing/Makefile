main: main.c
	arm-linux-gcc -o i2c_sensors main.c -lpthread -lrt -D_BSD_SOURCE -lm -I.

light: LightSensor.c
	arm-linux-gcc -o LightSensor LightSensor.c -lpthread -lrt -D_BSD_SOURCE -lm -I.

temp: TempSensor.c
	arm-linux-gcc -o TempSensor TempSensor.c -lpthread -lrt -D_BSD_SOURCE -lm -I.
