#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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

int main(int argc, char *argv[])
{

    char *targetIP = "10.0.0.66";
    int port = 8080;

    if(argc != 3){
      printf("Wrong # of arguments, using default IP (10.0.0.66) and Port (8080)\n");

    } else{
      targetIP   = argv[1];
      port       = atoi(argv[2]);  
    }

    printf("IP Address: %s\n", targetIP);
    printf("Port: %d\n", port);

    volatile unsigned int *fifo_periph = get_a_pointer(FIFO_PERIPH_ADDRESS);	
        unsigned int r_cnt = 0;
        unsigned int r_val;
        unsigned int fifo_occupancy = *(fifo_periph+FIFO_RX_OCCUPANCY_REG_OFFSET);

        // Setup & Create UDP socket
        int sockfd;
        struct sockaddr_in servaddr;
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        memset(&servaddr, 0, sizeof(servaddr));

        //build udp msg
        unsigned char myArray[1028];
        size_t myArraySize = sizeof(myArray); 
        unsigned int nread = 256; //read 256 words per packet - 1024 bytes + 4 bytes of header
        unsigned int packetidx = 0; 
        uint8_t* tmpword;
    
        printf("Starting Packet Generation");
        while(1){      
            r_cnt = 0;
            packetidx++;
            // printf("building pack et: %d\n", packetidx);

            // Cast the 32-bit word into an array of 4 uint8_t
            tmpword = (uint8_t*)&packetidx;

            // First 4 bytes of message are the packet number - Little Endian
            myArray[0] = tmpword[0];
            myArray[1] = tmpword[1];
            myArray[2] = tmpword[2];
            myArray[3] = tmpword[3];

            while(r_cnt < nread){
                fifo_occupancy = *(fifo_periph+FIFO_RX_OCCUPANCY_REG_OFFSET);
                if(fifo_occupancy > 0){
                    // Read from FIFO
                    r_val = *(fifo_periph+FIFO_RX_DATA_REG_OFFSET);
                    r_cnt++;
                    
                    // add to packet
                    tmpword = (uint8_t*)&r_val;
                    myArray[((r_cnt)*4)+0] = tmpword[0];
                    myArray[((r_cnt)*4)+1] = tmpword[1];
                    myArray[((r_cnt)*4)+2] = tmpword[2];
                    myArray[((r_cnt)*4)+3] = tmpword[3];
                    
                    // printf("Sample %d = %u (bytes %u %u %u %u)\n", r_cnt, r_val, ((r_cnt)*4)+0, ((r_cnt)*4)+1, ((r_cnt)*4)+2, ((r_cnt)*4)+3);
                }
                
                // empty_cnt++;
                // printf("Ocp = %u\n", fifo_occupancy );
                // printf("Lst = %u\n", r_val );
                // printf("Cnt = %u\n", r_cnt );
            }

            // printf("sending packet: %d\n", packetidx);
            // Filling server information
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(port);
            inet_pton(AF_INET, targetIP, &servaddr.sin_addr);

            // Send UDP packet
            sendto(sockfd, myArray, myArraySize, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

            // printf("sent packet: %d\n", packetidx);

            // for(int j = 0; j < myArraySize; j=j+4){
            //     printf("%02x %02x %02x %02x \n", myArray[j+0], myArray[j+1], myArray[j+2], myArray[j+3]);
            // }
        }
    return 0;
}
