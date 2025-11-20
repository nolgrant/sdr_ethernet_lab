#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <time.h>
#define _BSD_SOURCE

#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET 0
#define RADIO_TUNER_TUNER_PINC_OFFSET 1
#define RADIO_TUNER_CONTROL_REG_OFFSET 2
#define RADIO_TUNER_TIMER_REG_OFFSET 3
#define RADIO_PERIPH_ADDRESS 0x43c00000

#define FIFO_PERIPH_ADDRESS 0x43c10000
#define FIFO_RX_RESET_REG_OFFSET 6
#define FIFO_RX_OCCUPANCY_REG_OFFSET 7
#define FIFO_RX_DATA_REG_OFFSET 8

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
	return (radio_base);
}

void radioTuner_tuneRadio(volatile unsigned int *ptrToRadio, float tune_frequency)
{
	float pinc = (-1.0*tune_frequency)*(float)(1<<27)/125.0e6;
	*(ptrToRadio+RADIO_TUNER_TUNER_PINC_OFFSET)=(int)pinc;
  //printf("Tuned to: %f Hz (%i Phase Inc)\n", tune_frequency, (int)pinc);
}

void radioTuner_setAdcFreq(volatile unsigned int* ptrToRadio, float freq)
{
	float pinc = freq*(float)(1<<27)/125.0e6;
	*(ptrToRadio+RADIO_TUNER_FAKE_ADC_PINC_OFFSET) = (int)pinc;
  //printf("Signal at: %f Hz (%i Phase Inc)\n", freq, (int)pinc);
}

int main(int argc, char *argv[])
{

    //printf("C main function called from Python\n");
    //printf("Number of arguments: %d\n", argc);
    //for (int i = 0; i < argc; i++) {
    //    printf("Argument %d: %s\n", i, argv[i]);
    //}
    
    float fakeadc_freq = atof(argv[1]);
    //printf("FAKEADC_FREQ = %f\n", fakeadc_freq);
    
    float center_freq = atof(argv[2]);
    //printf("CENTER_FREQ = %f\n", center_freq);
    
    // first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    volatile unsigned int *radio_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);	
    volatile unsigned int *fifo_periph = get_a_pointer(FIFO_PERIPH_ADDRESS);	
    
    // printf("\n\r\n\r\n\rLab 11 Nolan Andreassen\n\r");

    // non-zero tune
    radioTuner_tuneRadio(radio_periph,center_freq);
    radioTuner_setAdcFreq(radio_periph,fakeadc_freq);

//    radioTuner_tuneRadio(radio_periph,30e6);
//    radioTuner_setAdcFreq(radio_periph,1760.0+30e6);

    // printf("Tuned to 1760Hz \n\r");

    // Disable output of Radio, Radio not in reset
    // printf("Disabling writes to FIFO\n\r");
    *(radio_periph+RADIO_TUNER_CONTROL_REG_OFFSET) = 0;

    // read fifo occupancy
    unsigned int fifo_occupancy = *(fifo_periph+FIFO_RX_OCCUPANCY_REG_OFFSET);
    unsigned int r_val;

    // clear fifo
    // printf("FIFO Occupancy = %u\n\r", fifo_occupancy ); 
    // printf("Clearing ...\n\r"); 
    for (int i=0;i<fifo_occupancy;i++)
        r_val = *(fifo_periph+FIFO_RX_DATA_REG_OFFSET);

    fifo_occupancy = *(fifo_periph+FIFO_RX_OCCUPANCY_REG_OFFSET);
    // printf("FIFO Occupancy = %u\n\r", fifo_occupancy );
    
    unsigned int nread = 480000;
    printf("Going to read %u samples\n\r", nread);

    // setup timed read
    unsigned int r_cnt = 0;
    unsigned int empty_cnt = 0;
    clock_t start_time, end_time;

    // Enable output of Radio (not in reset)
    *(radio_periph+RADIO_TUNER_CONTROL_REG_OFFSET) = 2; 
    
    fifo_occupancy = *(fifo_periph+FIFO_RX_OCCUPANCY_REG_OFFSET);
    start_time = clock();
    while(r_cnt < nread){
        if(fifo_occupancy > 0){
            for (int i=0;i<fifo_occupancy;i++){
                r_val = *(fifo_periph+FIFO_RX_DATA_REG_OFFSET);
                r_cnt++;
            }
        }
        fifo_occupancy = *(fifo_periph+FIFO_RX_OCCUPANCY_REG_OFFSET);
        empty_cnt++;
        // printf("Ocp = %u\n", fifo_occupancy );
        // printf("Lst = %u\n", r_val );
        // printf("Cnt = %u\n", r_cnt );
    }

    end_time = clock();
    printf("Finished reading\n\r");

    double time_spent_s = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    float samplesPerSec = ((float)r_cnt) / time_spent_s; 

    printf("Elapsed time = %2.3f seconds\n\r",time_spent_s);    
    printf("Measured Sample Rate = %2.4f ksps \n\r",samplesPerSec*(1e-3));
    printf("Expected Sample Rate = 48.8281 ksps \n\r");

    radioTuner_tuneRadio(radio_periph,0);
    radioTuner_setAdcFreq(radio_periph,0);

    printf("\n\r\n\r\n\r");
    return 0;
}
