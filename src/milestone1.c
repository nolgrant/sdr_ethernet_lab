#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{

  printf("\n\r\n\r\n\rLab 11 Nolan Andreassen - Milestone 1\n\r");

  char *targetIP = "10.0.0.66";
  int port = 8080;
  int numPackets = 100;

  // printf("Num Args = %i\n", argc);
  if (argc != 4)
  {
    printf("Wrong # of arguments, using default <IP> <Port> <NumPackets>\n");
    printf("Using defaults: IP=10.0.0.66, Port=8080, NumPackets=100\n");
  }
  else
  {
    targetIP = argv[1];
    port = atoi(argv[2]);
    numPackets = atoi(argv[3]);
  }

  printf("Sending %d Packets to: %s:%d\n", numPackets, targetIP, port);

  int sockfd;
  struct sockaddr_in servaddr;

  // Create UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0)
  {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Build Message
  unsigned char myArray[1028];
  size_t myArraySize = sizeof(myArray);
  uint32_t data = 0;
  uint8_t *tmpword;

  for (uint32_t i = 0; i < numPackets; i++)
  {
    // Cast the address of the 32-bit word to a pointer to an array of 4 uint8_t
    tmpword = (uint8_t *)&i;

    // First 4 bytes of message are the packet number - Little Endian
    myArray[0] = tmpword[0];
    myArray[1] = tmpword[1];
    myArray[2] = tmpword[2];
    myArray[3] = tmpword[3];

    // Fill remaining bytes with counting pattern (little endian)
    for (int j = 4; j < myArraySize; j = j + 4)
    {
      tmpword = (uint8_t *)&data;
      myArray[j + 0] = tmpword[3];
      myArray[j + 1] = tmpword[2];
      myArray[j + 2] = tmpword[1];
      myArray[j + 3] = tmpword[0];
      data++;
    }

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, targetIP, &servaddr.sin_addr);

    // Send UDP packet
    sendto(sockfd, myArray, myArraySize, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  }

  printf("Sent %d UDP packets to %s:%d\n", numPackets, targetIP, port);

  // Close the socket
  close(sockfd);

  printf("\n\r\n\r\n\r");
  return 0;
}