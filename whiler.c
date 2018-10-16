//header file
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>

#include <linux/spi/spidev.h>
#include <linux/i2c-dev.h>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/io.h>

#include "kbhit.h"
//

//define
#define I2C_DEV	"/dev/i2c-1"
#define SPI_DEV	"/dev/spidev0.0"
#define CLOCK_FREQ 	25000000.0
#define PCA_ADDR	0x40

#define MODE1 		0x00
#define MODE2 		0x01

#define LED0_ON_L 	0x06
#define LED0_OFF_L 	0x08

#define PRE_SCALE 	0xFE

#define GPIO_ADDRESS	0x7E200000
#define GPIO_IO_PERI	0x7E000000
#define BCM_IO_BASE	0x3F000000
#define GPIO_BASE	BCM_IO_BASE + (GPIO_ADDRESS - GPIO_IO_PERI)
#define GPIO_SIZE	(1024 * 4)

#define GPIO_IN(n)	(*(gpio+(n/10))&=~(7<<((n%10)*3)))
#define GPIO_OUT(n)	(*(gpio+(n/10))|=(1<<((n%10)*3)))
#define GPIO_SET(n)	(*(gpio+7)=(1<<n))
#define GPIO_CLR(n)	(*(gpio+10)=(1<<n))                                                     
#define GPIO_GET(n)	((*(gpio+13)&(1<<n))>>n)

#define SPI_CS		8
#define BLDC_ST	17
#define BLDC_SP	27
#define BLDC_CW	23
#define BLDC_CCW	22

#define LOW             0
#define HIGH            1

#define SPI_SPEED   	3000000  // 1MHza

#define SLEEP_SPEED	40000
#define SET_SPEED	100000
//

//global variable
pthread_mutex_t spi_Lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t snd_Lock = PTHREAD_MUTEX_INITIALIZER;

static unsigned char	spiBPW    = 8;
static unsigned short	spiDelay  = 0;

static volatile unsigned int *gpio;

static int spiFd;
static int i2cFd;
static int senFd;
static int memFd;

static unsigned char EMG_Channel[2] = {1, 2};
static unsigned char flex_Channel = 0;

char EMG_Check = 1;
char flex_Check = 1;

const float VCC = 4.98;
const float R_DIV = 47500.0;

volatile int angle;
unsigned int emg1v;
unsigned int emg2v;

int emg1_Ad;
int emg2_Ad;

int emg1_Ad_count;
int emg2_Ad_count;

int emg1_Pw;
int emg2_Pw;

float emg_pwm;

unsigned char buffer[3];
char snd_buff[3];

int stop;

int step;
//

//function
int read_mcp3008_adc(unsigned char adcChannel);
void SPIDataRW (unsigned char *data, int len);
float map(int x, int in_min, int in_max, int out_min, int out_max);
void *EMG_Function1(void *arg);
void *EMG_Function2(void *arg);
void *flex_Function(void *arg);
int reg_read8(int addr);
int reg_read16(unsigned char addr);
int reg_write8(unsigned char addr, unsigned char data);
int reg_write16(unsigned char addr, unsigned short data);
int pca9685_restart(void);
int pca9685_freq(void);
void servoOFF(void);
int BLDC_Motor(int pwm);
int init_func(void);
void close_func(void);
int calm_Adjustment(void);
int power_Adjustment(void);
int Calculate_Percentage(void);
//

int main(int argc, char *argv[])
{
	int adcEMG_Value;
	int adcAngle_Value;
	int input_pwm;
	int sub1;
	int sub2;

	pthread_t EMG_t[2], flex_t;

	char command;

	if(init_func())
		printf("SYSTEM ERROR init_func()\n");

	if(calm_Adjustment())
		printf("SYSTEM ERROR calm_Adjustment\n");

	if(power_Adjustment())
		printf("SYSTEM ERROR power_Adjustment\n");

	if(Calculate_Percentage())
		printf("SYSTEM ERROR Calculate_Percentage\n");

	sleep(10);

	//return 0;
	
	//create thread
	pthread_create(&flex_t, NULL, flex_Function, (void *)&flex_Channel);
	pthread_create(&EMG_t[0], NULL, EMG_Function1, (void *)&flex_Channel);
	pthread_create(&EMG_t[1], NULL, EMG_Function2, (void *)&flex_Channel);

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

		//return 0;

		if(angle > 63)
		{
			stop = 1;

			GPIO_SET(BLDC_SP);

			printf("Lower your arm.\n");

			BLDC_Motor(4095);
			GPIO_CLR(BLDC_SP);
			GPIO_CLR(BLDC_CW);

			while(angle > 36);

			stop = 0;
			printf("Upper your arm.\n");
			GPIO_SET(BLDC_CW);
		}
	}

END:
	EMG_Check = 0;
	flex_Check = 0;
	
	GPIO_SET(BLDC_SP);

	pthread_join(EMG_t[0], NULL);
	pthread_join(EMG_t[1], NULL);
	pthread_join(flex_t, NULL);

	pthread_mutex_destroy(&spi_Lock);

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

float map(int x, int in_min, int in_max, int out_min, int out_max)
{
	return (x - in_min) * ((float)(out_max - out_min) / (in_max - in_min)) + out_min;
}

void send_Data(void)
{
	pthread_mutex_lock(&snd_Lock);
	/*
	step++;
	
	if(step == 2)
	{
		sprintf(snd_buff, "%.f\t%d\t%d\n", angle, emg1v, emg2v);
		write(senFd, snd_buff, strlen(snd_buff));
		step = 0;
	}
	*/
	pthread_mutex_unlock(&snd_Lock);
}

void *EMG_Function1(void *arg)
{
	time_t now_time;
	int tmp = 300;
	int chk = 1;
	unsigned char adSign = *(unsigned char *)arg;

	while(adSign == 1 && EMG_Check)
	{
		emg1_Ad += read_mcp3008_adc(EMG_Channel[0]);
		printf("emg1_Ad : %d\n", emg1_Ad);
		emg1_Ad_count++;

		usleep(SET_SPEED);
	}

	while(adSign == 2 && EMG_Check)
	{
		int tmp = read_mcp3008_adc(EMG_Channel[0]);
		//printf("emg1 : %d\n", tmp);
		emg1_Pw = tmp > emg1_Pw ? tmp : emg1_Pw;

		usleep(SET_SPEED);
	}

	while(!adSign && EMG_Check)
	{
		while(stop);
		if(angle < tmp)
			chk = 0;
		else
			chk = 1;
		emg1v = read_mcp3008_adc(EMG_Channel[0]);
		if(chk && angle >0)
		{
			printf("EMG%d : %d\n", EMG_Channel[0], emg1v);
			send_Data();
		}
		tmp = angle;

		usleep(SLEEP_SPEED);
	}
}

void *EMG_Function2(void *arg)
{
	time_t now_time;
	int tmp = 300;
	int chk = 1;
	unsigned char adSign = *(unsigned char *)arg;

	while(adSign == 1 && EMG_Check)
	{
		emg2_Ad += read_mcp3008_adc(EMG_Channel[1]);
		printf("emg2_Ad : %d\n", emg2_Ad);
		emg2_Ad_count++;

		usleep(SET_SPEED);
	}

	while(adSign == 2 && EMG_Check)
	{
		int tmp = read_mcp3008_adc(EMG_Channel[1]);
		//printf("emg2 : %d\n", tmp);
		emg2_Pw = tmp > emg2_Pw ? tmp : emg2_Pw;

		usleep(SET_SPEED);
	}

	while(!adSign && EMG_Check)  
	{
		while(stop);
		if(angle < tmp)
			chk = 0;
		else
			chk = 1;
		emg2v = read_mcp3008_adc(EMG_Channel[1]);
		if(chk && angle >0)
		{
			printf("EMG%d : %d\n", EMG_Channel[1], emg2v);
			send_Data();		
		}
		tmp = angle;

		usleep(SLEEP_SPEED);
	}
}
void *flex_Function(void *arg)
{
	int value;
	time_t now_time;

	while(flex_Check)
	{
		angle = read_mcp3008_adc(flex_Channel);
		printf("flex angle: %d\n", angle);
		
		//angle = map(value, 30, 55, 0, 130);	

		//printf("flex angle : %.f\n\n",  angle);
		usleep(SLEEP_SPEED);
	}
}

int reg_read8(int addr)
{
	int length = 1;
	buffer[0] = addr;
	if(write(i2cFd, buffer, length) != length)
	{
		printf("Failed to write from to i2c bus\n");
	}
	if(read(i2cFd, buffer, length) != length)
	{
		printf("Failed to read from to i2c bus\n");
	}
	printf("addr[%d] = %d\n", addr, buffer[0]);
}

int reg_read16(unsigned char addr)
{
	unsigned short temp;

	reg_read8(addr);
	temp = 0xff & buffer[0];

	reg_read8(addr+1);
	temp |= (buffer[0] << 8);
	//printf("addr = 0x%x. data = 0x%x\n", addr, temp);
	return 0;
}

int reg_write8(unsigned char addr, unsigned char data)
{
	int length=2;

	buffer[0] = addr;
	buffer[1] = data;

	if(write(i2cFd, buffer, length)!=length)
	{
		printf("Failed to write from the i2c bus\n");
		return -1;
	}
	return 0;
}

int reg_write16(unsigned char addr, unsigned short data)
{
	reg_write8(addr, (data & 0xff));
	reg_write8(addr+1, (data>>8) & 0xff);
	return 0;
}

int pca9685_restart(void)
{	
	reg_write8(MODE1, 0x00);
	reg_write8(MODE2, 0x04);
	return 0;
}

int pca9685_freq(void)
{
	int freq = 50;
	uint8_t pre_val = (CLOCK_FREQ / 4096 / freq) -1; 
	printf("prescale_val = %d\n", pre_val);

	reg_write8(MODE1, 0x10);				// OP : OSC OFF
	reg_write8(PRE_SCALE, pre_val);				// OP : WRITE PRE_SCALE VALUE
	reg_write8(MODE1, 0x80);				// OP : RESTART
	reg_write8(MODE2, 0x04);				// OP : TOTEM POLE 

	reg_read8(PRE_SCALE);

	return 0;
}

int BLDC_Motor(int pwm)
{
	reg_write16(LED0_ON_L, 0);
	//reg_read16(LED0_ON_L);
	reg_write16(LED0_OFF_L, pwm);
	//reg_read16(LED0_OFF_L);

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

	if((senFd = open("sensor.txt", O_RDWR | O_CREAT | O_APPEND, 666)) < 0)
	{
		printf("SYSTEM ERROR open text\n");
		return -1;
	}

	if((i2cFd = open(I2C_DEV, O_RDWR))<0)
	{
		printf("Failed open i2c-1 bus\n");
		return -1;
	}

	if(ioctl(i2cFd,I2C_SLAVE,PCA_ADDR)<0)
	{
		printf("Failed to acquire bus access and/or talk to slave\n");
		return -1;
	}

	pca9685_restart();
	pca9685_freq();

	GPIO_OUT(SPI_CS);
	GPIO_OUT(BLDC_ST);
	GPIO_OUT(BLDC_SP);
	GPIO_OUT(BLDC_CW);					
	//GPIO_OUT(BLDC_CCW);
	
	GPIO_SET(BLDC_ST);
	GPIO_SET(BLDC_SP);
	
	return 0;
}

void close_func(void)
{
	reg_write8(MODE1, 0x10); //motor off

	close(memFd);
	close(spiFd);
	close(senFd);
	close(i2cFd);
}

int calm_Adjustment(void)
{
	pthread_t EMG_t[2];
	int time = 1;

	printf("Do not move and talk for 10 seconds.\nStart measurement.\n");
	sleep(3);

	pthread_create(&EMG_t[0], NULL, EMG_Function1, (void *)&EMG_Channel[0]);
	pthread_create(&EMG_t[1], NULL, EMG_Function2, (void *)&EMG_Channel[0]);

	
	while(time < 11)
	{
		printf("%2d second\n", time++);
		sleep(1);
	}

	printf("Measurement is completed.\n");
	EMG_Check = 0;

	pthread_join(EMG_t[0], NULL);
	pthread_join(EMG_t[1], NULL);

	EMG_Check = 1;

	return 0;
}

int power_Adjustment(void)
{
	pthread_t EMG_t[2], flex_t;

	printf("Use the force according to the speed of the motor.\n");
	sleep(3);

	pthread_create(&EMG_t[0], NULL, EMG_Function1, (void *)&EMG_Channel[1]);
	pthread_create(&EMG_t[1], NULL, EMG_Function2, (void *)&EMG_Channel[1]);
	pthread_create(&flex_t, NULL, flex_Function, NULL);

	BLDC_Motor(4095);
	GPIO_CLR(BLDC_SP);
	GPIO_SET(BLDC_CW);

	//printf("st %d sp %d cw %d\n", GPIO_GET(BLDC_ST), GPIO_GET(BLDC_SP), GPIO_GET(BLDC_CW));
	while(angle < 57);

	EMG_Check = 0;

	GPIO_SET(BLDC_SP);

	pthread_join(EMG_t[0], NULL);
	pthread_join(EMG_t[1], NULL);

	EMG_Check = 1;

	printf("Measurement is completed.\nWhile my arms are going down, Stay calm.\n");
	sleep(3);
	
	GPIO_CLR(BLDC_CW);
	GPIO_CLR(BLDC_SP);

	//printf("st %d sp %d cw %d ccw %d\n", GPIO_GET(BLDC_ST), GPIO_GET(BLDC_SP), GPIO_GET(BLDC_CW), GPIO_GET(BLDC_CCW));
	
	while(angle > 32);

	flex_Check = 0;

	GPIO_SET(BLDC_SP);

	pthread_join(flex_t, NULL);
	flex_Check = 1;

	printf("Setting is End\n");
	return 0;
}

int Calculate_Percentage(void)
{
	int emg1_range, emg2_range;
	int Total_emg;
	
	emg1_Ad = emg1_Ad / emg1_Ad_count;
	emg2_Ad = emg2_Ad / emg2_Ad_count;

	emg1_range = emg1_Pw - emg1_Ad;
	emg2_range = emg2_Pw - emg2_Ad;
	
	Total_emg = emg1_range + emg2_range;
	
	emg_pwm = (float)4095 / Total_emg;
	
	return 0;
}
