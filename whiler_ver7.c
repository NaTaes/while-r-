//header file
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>

#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "kbhit.h"
//

//define
#define I2C_DEV  	"/dev/i2c-1"
#define SPI_DEV		"/dev/spidev0.0"
#define CLOCK_FREQ 	25000000.0
#define PCA_ADDR	0x40
#define MPU6050_ADDR	0x68

#define MODE1 		0x00
#define MODE2 		0x01

#define LED0_ON_L 	0x06
#define LED0_OFF_L 	0x08

#define PRE_SCALE 	0xFE

#define GPIO_ADDRESS	0x7E200000
#define GPIO_IO_PERI	0x7E000000
#define BCM_IO_BASE	0x3F000000
#define GPIO_BASE       BCM_IO_BASE + (GPIO_ADDRESS - GPIO_IO_PERI)
#define GPIO_SIZE	(1024 * 4)

#define GPIO_IN(n)	(*(gpio+(n/10))&=~(7<<((n%10)*3)))
#define GPIO_OUT(n)	(*(gpio+(n/10))|=(1<<((n%10)*3)))
#define GPIO_SET(n)	(*(gpio+7)=(1<<n))
#define GPIO_CLR(n)	(*(gpio+10)=(1<<n))                                                     
#define GPIO_GET(n)	((*(gpio+13)&(1<<n))>>n)

#define SPI_CS		8
#define BLDC_ST 	17
#define BLDC_SP		27
#define BLDC_CW		22

#define PWR_MGMT_1 	0x6B
#define SMPLRT_DIV	0x19
#define CONFIG		0x1A
#define GYRO_CONFIG	0x1B
#define ACCEL_CONFIG	0x1C
#define INT_ENABLE	0x38
#define ACCEL_XOUT_H	0x3B
#define ACCEL_YOUT_H	0x3D
#define ACCEL_ZOUT_H	0x3F

#define LOW             0
#define HIGH            1

#define SPI_SPEED   	1000000  // 1MHza

#define SLEEP_SPEED	500000
#define SET_SPEED	100000

#define BUFF_SIZE 254
//

//global variable
pthread_mutex_t spi_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t i2c_Lock = PTHREAD_MUTEX_INITIALIZER;

static unsigned char	spiBPW    = 8;
static unsigned short	spiDelay  = 0;

static volatile unsigned int *gpio;

static int spiFd;
static int i2cFd;
static int senFd;
static int memFd;

static unsigned char EMG_Channel[3] = {0, 1, 2};

char EMG_Check = 1;
char GYRO_Check = 1;

unsigned int emg1v;
unsigned int emg2v;

int emg1_Ad;
int emg2_Ad;

int emg_Ad_count;

int emg1_Pw;
int emg2_Pw;

int emg_pwm;

int stop;
int step;

volatile float accel_angle_x;
volatile float baseAcX, baseAcY, baseAcZ;

int sock;
struct sockaddr_in server_addr;
char socket_buff[BUFF_SIZE];
//

//function
int read_mcp3008_adc(unsigned char adcChannel);
void SPIDataRW (unsigned char *data, int len);
void *EMG_Function(void *arg);
void *GYRO_Function(void *arg);
short reg_read8(unsigned char addr);
short reg_read16(unsigned char addr);
int reg_write8(unsigned char addr, unsigned char data, char module);
int reg_write16(unsigned char addr, unsigned short data, char module);
int pca9685_restart(void);
int pca9685_freq(void);
int BLDC_Motor(int pwm);
int init_func(void);
void close_func(void);
int calm_Adjustment(void);
int power_Adjustment(void);
int Calculate_Percentage(void);
int MPU6050_init(void);
short read_mpu6050_data(unsigned char addr);
int calibAccelGyro(void);
int server_connect(void);
//

int main(int argc, char *argv[])
{
	int adcEMG_Value;
	int adcAngle_Value;
	int input_pwm;
	int sub1, sub2;

	pthread_t EMG_t, Gyro_t;

	char command;

	if(init_func())
		printf("SYSTEM ERROR init_func()\n");

	if(MPU6050_init())
		printf("SYSTEM ERROR MPU6050_init\n");
	
	if(calibAccelGyro())
		printf("SYSTEM ERROR calibAccelGyro\n");
	
	if(calm_Adjustment())
		printf("SYSTEM ERROR calm_Adjustment\n");

	if(power_Adjustment())
		printf("SYSTEM ERROR power_Adjustment\n");

	if(Calculate_Percentage())
		printf("SYSTEM ERROR Calculate_Percentage\n");

	if(server_connect())
	{
		printf("SYSTEM ERROR can't Server connect\n");
		return -1;
	}

	printf("set ok let's go\n");
	sleep(5);

		
	//create thread
	pthread_create(&EMG_t, NULL, EMG_Function, (void *)&EMG_Channel[2]);
	pthread_create(&Gyro_t, NULL, GYRO_Function, (void *)&EMG_Channel[2]);


	BLDC_Motor(0);
	GPIO_CLR(BLDC_SP);
	GPIO_SET(BLDC_CW);

	while(1)
	{
		if(kbhit())
		{
			switch(getchar()){
				case 'b' :
					goto END;
					break;

			}
		}

		sub1 = emg1v - emg1_Ad;
		sub2 = emg2v - emg2_Ad;
		
		//motor control
		input_pwm = (int)((sub1 < 0 ? 0 : sub1 > (emg1_Pw - emg1_Ad) ? (emg1_Pw - emg1_Ad) : sub1) * emg_pwm);
	  	input_pwm += (int)((sub2 < 0 ? 0 : sub2 > (emg2_Pw - emg2_Ad) ? (emg2_Pw - emg2_Ad) : sub2) * emg_pwm);
		
		printf("emg1v : %d emg1_Ad : %d emg2v : %d emg2_Ad : %d emg1_Pw : %d emg2_Pw : %d pwm = %d\n", emg1v, emg1_Ad, emg2v, emg2_Ad, emg1_Pw, emg2_Pw, input_pwm);

		BLDC_Motor(input_pwm);
		usleep(SLEEP_SPEED);

		if(accel_angle_x > 7.0)
		{
			stop = 1;

			GPIO_SET(BLDC_SP);

			printf("Lower your arm.\n");

			BLDC_Motor(4095);
			GPIO_CLR(BLDC_SP);
			GPIO_CLR(BLDC_CW);

			while(accel_angle_x > 0.0);

			stop = 0;
			printf("Upper your arm.\n");
			GPIO_SET(BLDC_CW);
		}
	}

END:
	EMG_Check = 0;
	GYRO_Check = 0;

	GPIO_SET(BLDC_SP);

	pthread_join(EMG_t, NULL);
	pthread_join(Gyro_t, NULL);

	pthread_mutex_destroy(&spi_Lock);
	pthread_mutex_destroy(&i2c_Lock);

	close_func();

	return 0;
}

//function part
void SPIDataRW(unsigned char *data, int len)
{
	struct spi_ioc_transfer spi;
	int output;

	memset(&spi, 0, sizeof(spi));

	spi.tx_buf        = (unsigned long)data;
	spi.rx_buf        = (unsigned long)data;
	spi.len           = len;
	spi.delay_usecs   = spiDelay;
	spi.speed_hz      = SPI_SPEED;
	spi.bits_per_word = spiBPW;

	if((output = ioctl(spiFd, SPI_IOC_MESSAGE(1), &spi)) == -1)
		printf("ERROR ioctl\n");
}

int read_mcp3008_adc(unsigned char adcChannel)
{
	pthread_mutex_lock(&spi_Lock);

	unsigned char buff[3];
	int adcValue;

	buff[0] = 0x01; //start bit
	buff[1] = ((adcChannel | 0x08) << 4);
	buff[2] = 0x00;

	GPIO_CLR(SPI_CS);  // Low : CS Active

	SPIDataRW(buff, 3);
	buff[1] = 0x03 & buff[1];
	adcValue = (buff[1] << 8) | buff[2];

	GPIO_SET(SPI_CS);  // High : CS Inactive

	pthread_mutex_unlock(&spi_Lock);

	return adcValue;
}

void *EMG_Function(void *arg)
{
	int chk = 1;
	unsigned char adSign = *(unsigned char *)arg;
	
	if(adSign == 0)
	{
		while(EMG_Check)
		{
			emg1_Ad += read_mcp3008_adc(EMG_Channel[0]);
			emg2_Ad += read_mcp3008_adc(EMG_Channel[1]);
			//printf("emg1_Ad : %d\n", emg1_Ad);
			emg_Ad_count++;
	
			usleep(SET_SPEED);
		}
	}
	else if(adSign == 1)
	{
		while(EMG_Check)
		{
			int tmp1 = read_mcp3008_adc(EMG_Channel[0]);
			int tmp2 = read_mcp3008_adc(EMG_Channel[1]);
			//printf("emg1 : %d\n", tmp);
			emg1_Pw = tmp1 > emg1_Pw ? tmp1 : emg1_Pw;
			emg2_Pw = tmp2 > emg2_Pw ? tmp2 : emg2_Pw;
	
			usleep(SET_SPEED);
		}
	}
	else
	{
		float tmp = 0.0;
		while(EMG_Check)
		{
			while(stop);
			if(accel_angle_x < tmp)
				chk = 0;
			else
				chk = 1;
			emg1v = read_mcp3008_adc(EMG_Channel[0]);
			emg2v = read_mcp3008_adc(EMG_Channel[1]);
			printf("EMG%d : %d\n", EMG_Channel[0], emg1v);
			printf("EMG%d : %d\n", EMG_Channel[1], emg2v);
			if(chk && accel_angle_x >0)
			{
				printf("EMG%d : %d\n", EMG_Channel[0], emg1v);
				sprintf(socket_buff, "%s %d %d %.3f ", "hsh", emg1v, emg2v, accel_angle_x);
				write(sock, socket_buff, strlen(socket_buff)+1);
			}
			tmp = accel_angle_x;

			usleep(SLEEP_SPEED);
		}
	}
}

void *GYRO_Function(void *arg)
{
	float AcX, AcY, AcZ;
	float accel_x, accel_y, accel_z;
	float accel_xz;
	const float RADIANS_TO_DEGREES = 180 / 3.14159;

	while(GYRO_Check)
	{
		AcX = read_mpu6050_data(ACCEL_XOUT_H);
		AcY = read_mpu6050_data(ACCEL_YOUT_H);
		AcZ = read_mpu6050_data(ACCEL_ZOUT_H);
	
		accel_x = AcX - baseAcX;
		accel_y = AcY - baseAcY;
		accel_z = AcZ - (16384 - baseAcZ);
	
		accel_xz = sqrt(pow(accel_x, 2) + pow(accel_z, 2));
		accel_angle_x = atan2(accel_y, accel_xz) * RADIANS_TO_DEGREES;

		printf("angle = %.3f\n", accel_angle_x);

		usleep(SLEEP_SPEED);
	}
}

short reg_read8(unsigned char addr)
{
	unsigned char buffer;
	int length = 1;
	buffer = addr;
	
	pthread_mutex_lock(&i2c_Lock);

	if(ioctl(i2cFd, I2C_SLAVE, MPU6050_ADDR) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave MPU6050\n");
		return -1;
	}
	
	if(write(i2cFd, &buffer, length) != length)
	{
		printf("Failed to write from to i2c bus\n");
	}
	if(read(i2cFd, &buffer, length) != length)
	{
		printf("Failed to read from to i2c bus\n");
	}

	pthread_mutex_unlock(&i2c_Lock);

	return buffer;
}

short reg_read16(unsigned char addr)
{
	unsigned char buffer;
	unsigned short temp;

	buffer = reg_read8(addr);
	temp = 0xff & buffer;

	buffer = reg_read8(addr+1);
	temp |= (buffer << 8);
	return temp;
}

short read_mpu6050_data(unsigned char addr)
{
	short high_byte, low_byte, value;
	high_byte = reg_read8(addr);
	low_byte = reg_read8(addr+1);
	value = (high_byte << 8) | low_byte;
	return value;
}

int reg_write8(unsigned char addr, unsigned char data, char module)
{
	unsigned char buffer[2];
	int length=2;

	buffer[0] = addr;
	buffer[1] = data;

	pthread_mutex_lock(&i2c_Lock);

	if(module)
	{
		if(ioctl(i2cFd, I2C_SLAVE, PCA_ADDR) < 0)
		{
			printf("Failed to acquire bus access and/or talk to slave PCA9685\n");
			return -1;
		}
	}
	else
	{
		if(ioctl(i2cFd, I2C_SLAVE, MPU6050_ADDR) < 0)
		{
			printf("Failed to acquire bus access and/or talk to slave MPU6050\n");
			return -1;
		}
	}

	if(write(i2cFd, buffer, length)!=length)
	{
		printf("Failed to write from the i2c bus\n");
		return -1;
	}

	pthread_mutex_unlock(&i2c_Lock);

	return 0;
}

int reg_write16(unsigned char addr, unsigned short data, char module)
{
	reg_write8(addr, (data & 0xff), module);
	reg_write8(addr+1, (data>>8) & 0xff, module);
	return 0;
}

int pca9685_restart(void)
{	
	reg_write8(MODE1, 0x00, 1);
	reg_write8(MODE2, 0x04, 1);
	return 0;
}

int pca9685_freq(void)
{
	int freq = 50;
	uint8_t pre_val = (CLOCK_FREQ / 4096 / freq) -1; 
	printf("prescale_val = %d\n", pre_val);

	reg_write8(MODE1, 0x10, 1);				// OP : OSC OFF
	reg_write8(PRE_SCALE, pre_val, 1);				// OP : WRITE PRE_SCALE VALUE
	reg_write8(MODE1, 0x80, 1);				// OP : RESTART
	reg_write8(MODE2, 0x04, 1);				// OP : TOTEM POLE 

	//reg_read8(PRE_SCALE);
	return 0;
}

int BLDC_Motor(int pwm)
{
	reg_write16(LED0_ON_L, 0, 1);
	reg_write16(LED0_OFF_L, pwm, 1);

	return 0;
}

int init_func(void)
{
	if((memFd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
	{
		printf("SYSTEM ERROR open(/dev/mem)\n");
		return -1 ;
	}

	if((gpio = (unsigned int *)mmap(NULL, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memFd, GPIO_BASE)) == MAP_FAILED)
	{
		printf("SYSTEM ERROR gpio mmap\n");
		return -1 ;
	}

	if((spiFd = open(SPI_DEV, O_RDWR)) < 0)
	{
		printf("SYSTEM ERROR open(/dev/spidev0.0)\n");
		return -1;
	}

	if((i2cFd = open(I2C_DEV, O_RDWR))<0)
	{
		printf("Failed open i2c-1 bus\n");
		return -1;
	}

	pca9685_restart();
	pca9685_freq();

	GPIO_OUT(SPI_CS);
	GPIO_OUT(BLDC_ST);
	GPIO_OUT(BLDC_SP);
	GPIO_OUT(BLDC_CW);					
	
	GPIO_SET(BLDC_ST);
	GPIO_SET(BLDC_SP);
	
	return 0;
}

void close_func(void)
{
	reg_write8(MODE1, 0x10, 1); //motor off

	close(memFd);
	close(spiFd);
	close(senFd);
	close(i2cFd);
	close(sock);
}

int calm_Adjustment(void)
{
	pthread_t EMG_t;
	int time = 1;

	printf("Do not move and talk for 10 seconds.\nStart measurement.\n");

	pthread_create(&EMG_t, NULL, EMG_Function, (void *)&EMG_Channel[0]);
	
	while(time < 11)
	{
		printf("%2d second\n", time++);
		sleep(1);
	}

	printf("Measurement is completed.\n");
	EMG_Check = 0;

	pthread_join(EMG_t, NULL);

	EMG_Check = 1;

	return 0;
}

int power_Adjustment(void)
{
	int time = 3;
	int i;
	pthread_t EMG_t, Gyro_t;

	printf("Use the force according to the speed of the motor.\n");

	while(time > 0)
	{
		printf("%2d second\n", time--);
		sleep(1);
	}

	pthread_create(&EMG_t, NULL, EMG_Function, (void *)&EMG_Channel[1]);
	pthread_create(&Gyro_t, NULL, GYRO_Function, NULL);

	BLDC_Motor(4095);
	GPIO_CLR(BLDC_SP);
	GPIO_SET(BLDC_CW);

	while(accel_angle_x < 7.0);
	
	EMG_Check = 0;

	GPIO_SET(BLDC_SP);

	pthread_join(EMG_t, NULL);

	EMG_Check = 1;

	printf("Measurement is completed.\nWhile my arms are going down, Stay calm.\n");
	time = 3;
	while(time > 0)
	{
		printf("%2d second\n", time--);
		sleep(1);
	}
	
	GPIO_CLR(BLDC_CW);
	GPIO_CLR(BLDC_SP);

	while(accel_angle_x > 0.0);

	GYRO_Check = 0;

	GPIO_SET(BLDC_SP);

	pthread_join(Gyro_t, NULL);
	GYRO_Check = 1;

	printf("Setting is End\n");
	return 0;
}

int Calculate_Percentage(void)
{
	int emg1_range, emg2_range;
	int Total_emg;
	
	printf("emg1_Ad : %d(%d)\nemg2_Ad : %d(%d)\n", emg1_Ad, emg_Ad_count, emg2_Ad, emg_Ad_count);

	emg1_Ad = emg1_Ad / emg_Ad_count;
	emg2_Ad = emg2_Ad / emg_Ad_count;

	printf("Average emg1 : %d\nAverage emg2 : %d\n", emg1_Ad, emg2_Ad);
	printf("emg1_Pw : %d\nemg2_Pw : %d\n", emg1_Pw, emg2_Pw);

	emg1_range = emg1_Pw - emg1_Ad;
	emg2_range = emg2_Pw - emg2_Ad;

	printf("emg1_range : %d\nemg2_range : %d\n", emg1_range, emg2_range);
	
	Total_emg = emg1_range + emg2_range;

	emg_pwm = (float)4095 / Total_emg;
	
	printf("emg_pwm : %d\n", emg_pwm);
	sleep(2);

	return 0;
}

int MPU6050_init(void)
{
	reg_write8(SMPLRT_DIV, 0x07, 0);
	reg_write8(PWR_MGMT_1, 0x01, 0);
	reg_write8(CONFIG, 0x00, 0);
	reg_write8(GYRO_CONFIG, 0x18, 0);
	reg_write8(ACCEL_CONFIG, 0x18, 0);
	reg_write8(INT_ENABLE, 0x01, 0);

	return 0;
}

int calibAccelGyro(void)
{
	float sumAcX = 0, sumAcY = 0, sumAcZ = 0;
	float sumGyX = 0, sumGyY = 0, sumGyZ = 0;
	int i;

	for(i=0; i<100; i++)
	{
		sumAcX += read_mpu6050_data(ACCEL_XOUT_H);
		sumAcY += read_mpu6050_data(ACCEL_YOUT_H);
		sumAcZ += read_mpu6050_data(ACCEL_ZOUT_H);
		
		usleep(SET_SPEED);
	}

	baseAcX = sumAcX / i;
	baseAcY = sumAcY / i;
	baseAcZ = sumAcZ / i;

	return 0;
}

int server_connect(void)
{
	printf("socket make start\n");
	if((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return 1;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8003);
	server_addr.sin_addr.s_addr = inet_addr("192.168.1.24");

	if((connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1)
		return 1;

	printf("connect socket\n");
	return 0;
}
