#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[]){

    // printf("Num Args = %i\n", argc);

    if(argc != 4){
        printf("Arguments must be in the following format: <IP> <Port> <NumPackets>\n");
        printf("Example: \"./sendupdpacket 10.0.0.66 8080 100/\"");
        return 0;
    }

    char *targetIP = argv[1];
    int port       = atoi(argv[2]);
    int numPackets = atoi(argv[3]);
    // printf("IP Address: %s\n", targetIP);
    // printf("Number of packets: %d\n", numPackets);

    int sockfd;
    struct sockaddr_in servaddr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Build Message 
    unsigned char myArray[1028];
    size_t myArraySize = sizeof(myArray); 
    uint32_t data = 0;
    uint8_t* tmpword;

    for ( uint32_t i = 0; i < numPackets; i++){   
        // Cast the address of the 32-bit word to a pointer to an array of 4 uint8_t
        tmpword = (uint8_t*)&i;

        // First 4 bytes of message are the packet number - Little Endian
        myArray[0] = tmpword[3];
        myArray[1] = tmpword[2];
        myArray[2] = tmpword[1];
        myArray[3] = tmpword[0];

        // Fill remaining bytes with counting pattern (little endian)
        for(int j = 4; j < myArraySize; j=j+4){
            tmpword = (uint8_t*)&data;
            myArray[j+0] = tmpword[3];
            myArray[j+1] = tmpword[2];
            myArray[j+2] = tmpword[1];
            myArray[j+3] = tmpword[0];
            data++;
        }

        // Filling server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

        // Send UDP packet
        sendto(sockfd, myArray, myArraySize, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        
    }
    
    printf("Sent %d UDP packets to %s:%d\n", numPackets, targetIP, port);

    // Close the socket
    close(sockfd);

    return 0;
}