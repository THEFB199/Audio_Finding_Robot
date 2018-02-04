#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <stdio.h>
#include <stdlib.h>

#define sample_time 0.0100
#define buffer_length 8192
#include <time.h>
#include <iostream>
#include <complex>
#include <valarray>
#include <math.h>
using namespace std;
typedef complex<double> Complex;
typedef valarray<Complex> CArray;
const double PI = 3.141592653589793238460; // Pi constant with double precision

#define i2c_addr 0x50 // address of the mbed sudo i2cdetect -y 0

void connect(int * file_i2c);
void write_mbed(char data, int *file_i2c);
float read_float(int length, int *file_i2c);
void fft(CArray &x);
void data_collection (Complex *mic_data_1, Complex *mic_data_2, Complex *mic_data_3, int *file_i2c);
void power_calc(Complex *mic_data_1, Complex *mic_data_2, Complex *mic_data_3);
int index_pos(int n, int freq_low, int freq_high, int sample_freq)
int main(){
    int file_i2c;
    connect(&file_i2c); // establish connection with mbed
    write_mbed('1', &file_i2c); // initialise and synchronise data aquistion on mbed

    clock_t time;
	double duration;
	float r;
	cout << "Proggy Start\n";
    // complex arrays for each microphone for FFT done later
	Complex mic_data_1[buffer_length]={0}; // initialise to zero for ease of FFT 
	Complex mic_data_2[buffer_length]={0};
	Complex mic_data_3[buffer_length]={0};
	
	time = clock(); // begin timer
	
	data_collection (mic_data_1, mic_data_2, mic_data_3, &file_i2c);
	
	duration = (clock() - time)/ (double)CLOCKS_PER_SEC; // time run - note does not
    cout << " time taken:: " << duration << endl;
    // CArray needed for compatibility with FFT algorithm used.
	CArray data(mic_data_1, buffer_length);
	CArray data1(mic_data_2, buffer_length);
	CArray data2(mic_data_3, buffer_length);
	time = clock(); // begin timer
	// compute fft for this data
	fft(data);
	fft(data1);
	fft(data2);
    power_calc(mic_data_1, mic_data_2, mic_data_3);
	duration = (clock() - time) / (double)CLOCKS_PER_SEC; // time run - note does not seem to time data collection correctly
	cout << " time taken:: " << duration << endl;
    for (int i = 0; i < buffer_length; ++i)
    {
      cout << data[i] << endl;
    }
    //
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
	
	int addr = i2c_addr;          //<<<<<The I2C address of the slave
	if (ioctl(*file_i2c, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		//return;
	}
	//return *file_i2c;
}

float read_float(int length, int *file_i2c){
    char buffer1[10] = {0};
	//----- READ BYTES -----
	if (read(*file_i2c, buffer1, length) != length)		//read() returns the number of  bytes actually read, if it doesn't match then an error occurred (e.g. no response from the device)
	{
		//ERROR HANDLING: i2c transaction failed
		printf("Failed to read from the i2c bus.\n");
	}
	else
	{
        return(atof(buffer1)); // return floating point number for data later - data is recieved as a char. 
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

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
// Better optimized but less intuitive
// !!! Warning : in some cases this code make result different from not optimased version above (need to fix bug)
// The bug is now fixed @2017/05/30 
void fft(CArray &x)
{
	// DFT
	unsigned int N = x.size(), k = N, n;
	double thetaT = 3.14159265358979323846264338328L / N;
	Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
	while (k > 1)
	{
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for (unsigned int l = 0; l < k; l++)
		{
			for (unsigned int a = l; a < N; a += n)
			{
				unsigned int b = a + k;
				Complex t = x[a] - x[b];
				x[a] += x[b];
				x[b] = t * T;
			}
			T *= phiT;
		}
	}
	// Decimate
	unsigned int m = (unsigned int)log2(N);
	for (unsigned int a = 0; a < N; a++)
	{
		unsigned int b = a;
		// Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if (b > a)
		{
			Complex t = x[a];
			x[a] = x[b];
			x[b] = t;
		}
	}
	//// Normalize (This section make it not working correctly)
	//Complex f = 1.0 / sqrt(N);
	//for (unsigned int i = 0; i < N; i++)
	//	x[i] *= f;
}

void data_collection (Complex *mic_data_1, Complex *mic_data_2, Complex *mic_data_3, int *file_i2c){

    int i = 0;
    float mic1, mic2, mic3;
    //int 
    
    float done;

	
    // start data acqusition from mbed (due to no analogue inputs on pi)
	while (i < buffer_length) {

	    mic1 = read_float(10, file_i2c);
		mic_data_1[i] = (Complex) mic1;

	    mic2 = read_float(10, file_i2c);
        mic_data_2[i] = (Complex) mic2;


	    mic3 = read_float(10, file_i2c);
		mic_data_3[i] = (Complex) mic2;
		i++;
        done  = (float)i / (float)buffer_length; // how far through data collection are we?
        printf("Progress:: %f \r", done*100);
		//sleep (1);
	}

}

void power_calc(Complex *mic_data_1, Complex *mic_data_2, Complex *mic_data_3, int index_pos){
    
    double power_mic_1 = sqrt( pow(mic_data_1[index_pos].real(), 2.0) + pow(mic_data_1[index_pos].imag(), 2.0) );
    double power_mic_2 = sqrt( pow(mic_data_2[index_pos].real(), 2.0) + pow(mic_data_2[index_pos].imag(), 2.0) );
    double power_mic_3 = sqrt( pow(mic_data_3[index_pos].real(), 2.0) + pow(mic_data_3[index_pos].imag(), 2.0) );

    //cout << power_mic_1 << endl;

}

int index_pos(int n, int freq_low, int freq_high, int sample_freq){

    /* calculates index positions required for finding the specific
    frequency from the FFT. A range is presented here.
    This is because band pass filter centred around 2.4 kHz has width of 
    around 400 Hz 
    
    n = sample size
    sample_freq = sampling frequnecy used in data collection
    freq_low = lower frequency wanted
    freq_high = higher frequecny wanted
    */
    
    int lower_pos = n*(freq_low/sample_freq); 
    int upper_pos = n*(freq_high/sample_freq);
    
    return (lower_pos, upper_pos);

}
