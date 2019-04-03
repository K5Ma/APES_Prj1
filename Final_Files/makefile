all: Prj1 clean

#CC      = /home/k5m/buildroot/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-gcc.br_real

CFLAGS  = -lpthread -lrt -D_BSD_SOURCE -lm -I.

objects = main.o My_Time.o POSIX_Qs.o LoggingThread.o SocketThread.o TempThread.o LuxThread.o GPIO_PINs.o

khalid_Prj1: CC = /home/k5m/buildroot/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-gcc.br_real
khalid_Prj1: $(objects)
	$(CC) $(CFLAGS) -o Prj1 $(objects)

poorn_Prj1: CC = /home/poorn/Workspace/old_buildroot/buildroot/output/host/usr/bin/arm-buildroot-linux-uclibcgnueabihf-gcc.br_real
poorn_Prj1: $(objects)
	$(CC) $(CFLAGS) -o Prj1 $(objects)

main.o: Global_Defines.h My_Time.h POSIX_Qs.h LoggingThread.h SocketThread.h TempThread.h LuxThread.h GPIO_PINs.h
My_Time.o: My_Time.h
POSIX_Qs.o: POSIX_Qs.h Global_Defines.h My_Time.h
LoggingThread.o: LoggingThread.h Global_Defines.h My_Time.h POSIX_Qs.h
SocketThread.o: SocketThread.h Global_Defines.h POSIX_Qs.h
TempThread.o: TempThread.h Global_Defines.h POSIX_Qs.h
LuxThread.o: LuxThread.h Global_Defines.h POSIX_Qs.h
GPIO_PINs.o: GPIO_PINs.h

clean:
	rm -f  *o $(objects)

