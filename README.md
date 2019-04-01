# APES_Prj1
APES Project 1 - By Khalid AlAwadhi and Poorn Mehta 2019

In this project we created a multithreaded system on BeagleBone.
This system utilizes pthreads to connect to multiple offboard sensors, which are TMP102 (temperature sensor) and APDS-9301 (light sensor).
They will both share the same I2C bus. Other pthreads will be used log information and allow the board to be ready to receive commands from an external source as well as read log data from our system. 

More information can be found in our Project 1 Report file.