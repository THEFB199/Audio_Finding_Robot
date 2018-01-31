#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <stdio.h>
#include <stdlib.h>
void connect(int * file_i2c);
void write_mbed(char data, int *file_i2c);
float read_float(int length, int *file_i2c);

int main(){
    int file_i2c;
    float mic1, mic2, mic3;
	
	connect(&file_i2c);
	
	test = read_float(10, &file_i2c);
    printf("%f\n", test);

	write_mbed('a', &file_i2c);
}

void connect(int *file_i2c){
	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-0";
	if ((*file_i2c = open(filename, O_RDWR)) < 0)
	{   printf("%i\n", *file_i2c);
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus\n");
		//return;
	}
	
	int addr = 0x09;          //<<<<<The I2C address of the slave
	if (ioctl(*file_i2c, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		//return;
	}
	//return *file_i2c;
}

float read_float(int length, int *file_i2c){
    char buffer1[60] = {0};
	//----- READ BYTES -----
	//length = 10;			//<<< Number of bytes to read
	if (read(*file_i2c, buffer1, length) != length)		//read() returns the number of  bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
	{
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	else
	{
		printf("Data read: %s\n", buffer1);
        return(atof(buffer1));
	}
}

void write_mbed(char data, int *file_i2c){
    int length;
    	//----- WRITE BYTES -----
	//buffer[1] = 0x02;
    length = 1;			//<<< Number of bytes to write
	if (write(*file_i2c, &data, length) != length)		//write() returns the number of bytes actually written, if it doesn't match then an error occurred (e.g. no response from the device)
	{
		/* ERROR HANDLING: i2c transaction failed */
		printf("Failed to write to the i2c bus.\n");
	}

}


