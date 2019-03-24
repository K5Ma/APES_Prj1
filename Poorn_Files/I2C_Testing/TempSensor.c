#include "TempSensor.h"

int fd;
char *bus = "/dev/i2c-2";    // P9_19 and P9_20 
int addr = 0x48;             
char buf[2] = {0},pika=0x00;
int temp;
unsigned char MSB, LSB;
float f,c,k; 

typedef enum
{
	TEMP_CEL = 0,
	TEMP_KEL = 1,
	TEMP_FAH = 2
}temp_format;

int default_config_byte_one = 0x50;
int default_config_byte_two = 0xA0;

void write_pointer_reg(uint8_t value)
{
	if(write(fd, &value, 1) != 1)
	{
		perror("Write Pointer Register Error\n");
	}
}

int write_tlow_reg(int reg, uint16_t value )
{
	write_pointer_reg(reg);
	
	if (write(fd, &value, 2) != 2)
	{
		perror("T-low register write error");
		return -1;
	}
	return 0;
}


int write_config_reg_on_off(uint8_t value )
{
	write_pointer_reg(0b00000001);
	if((value == 0) || (value == 1))
	{
		default_config_byte_one |= value;
		
		if (write(fd, &default_config_byte_one, 1) != 1)
		{
			perror("Configuration register write error for first byte");
			return -1;
		}
		
		if (write(fd, &default_config_byte_two, 1) != 1)
		{
			perror("Configuration register write error for second byte");
			return -1;
		}
	}
	return 0;
}

int write_config_reg_em(uint8_t value )
{
	write_pointer_reg(0b00000001);
	if((value == 0) || (value == 1))
	{
		default_config_byte_two |= (value << 4);
		
		if (write(fd, &default_config_byte_one, 1) != 1)
		{
			perror("Configuration register write error for first byte");
			return -1;
		}
		
		if (write(fd, &default_config_byte_two, 1) != 1)
		{
			perror("Configuration register write error for second byte");
			return -1;
		}
	}
	return 0;
}

int write_config_reg_conv_rate(uint8_t value )
{
	write_pointer_reg(0b00000001);
	if((value >= 0) || (value <= 3))
	{
		default_config_byte_two |= (value << 6);
		if (write(fd, &default_config_byte_one, 1) != 1)
		{
			perror("Configuration register write error for first byte");
			return -1;
		}
		if (write(fd, &default_config_byte_two, 1) != 1)
		{
			perror("Configuration register write error for second byte");
			return -1;
		}
	}
	return 0;
}

int write_config_register_default( )
{
	write_pointer_reg(0b00000001);
	if (write(fd, &default_config_byte_one, 1) != 1)
	{
		perror("Configuration register write error for first byte");
		return -1;
	}
	if (write(fd, &default_config_byte_two, 1) != 1)
	{
		perror("Configuration register write error for second byte");
		return -1;
	}
	return 0;
}



uint16_t read_tlow_reg(int reg)
{
	uint16_t value;
	uint8_t v[1]={0};
	write_pointer_reg(reg);
	if (read(fd, v, 1) != 1)
	{
		perror("T-low register read error");
		return -1;
	}
	value = (v[0]<<4 | (v[1] >> 4 & 0XF));
	printf("T-low register value is: %d \n", value);
	return value;
}

uint16_t read_temp_config_register()
{
	uint16_t value;
	uint8_t v[1]={0};
	write_pointer_reg(0b00000001);
	if (read(fd, v, 1) != 1)
	{
		perror("Temperature configuration register read error");
		return -1;
	}
	value = (v[0]<<8 | v[1]);
	printf("Temperature configuration register value is: %d \n", value);
	return value;
}

int all_temprg_rd_wr()//useless
{
	if (write_config_reg_on_off(1) < 0)
	{
		return -1;
	}

	if (write_config_reg_em(1) < 0)
	{
		return -1;
	}

	if (read_temp_config_register() < 0)
	{
		return -1;
	}

	if (write_config_register_default() < 0)
	{
		return -1;
	}

	if (write_config_reg_conv_rate(2) < 0)
	{
		return -1;
	}

	if (write_tlow_reg(0x02,45) < 0)
	{
		return -1;
	}

	if (read_tlow_reg(0x02) < 0)
	{
		return -1;
	}

	return 0;

}

float read_temp_data_reg(int unit)
{

	write_pointer_reg(0b00000000);

	int x = read(fd,&buf,2);

	if (x != 2)
	{
		perror("Failed to read from the i2c bus.\n");
		printf("ERROR : %s\n", strerror(errno));
		return -300;
	}
	else
	{

		MSB = buf[0];
		LSB = buf[1];

		temp = ((MSB << 8) | LSB) >> 4;

		c = temp*0.0625;
		f = (1.8 * c) + 32;
		k = c + 273.15;
	
		if(unit == TEMP_CEL)
		return c;
		else if(unit == TEMP_KEL)
		return k;
		else if(unit == TEMP_FAH)
		return f;
	}
}

int temp_init()
{
	if((fd = open(bus, O_RDWR)) < 0)
	{
		perror("Failed to open the i2c bus");
		return -1;
	}

	if(ioctl(fd, I2C_SLAVE, addr) < 0)
	{
		perror("Failed to open the i2c bus");
		return -1;
	}

	return 0;
}
/*
int main(void)
{
	temp_init();
	while(1){
	float temp = read_temp_data_reg(0);
	printf("Current Temp: %f\n", temp);
	}
}*/
