#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv) {
   int    sockfd, n;
   char   recvline[MAXLINE + 1];
   char   error[MAXLINE + 1];
   struct sockaddr_in servaddr;

   // The second argument expected is the ipaddres for connection
   if (argc < 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress>");
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   // Creating the socket
   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket error");
      exit(1);
   }

   // converting the Port argument (argv[2]) from string to int
   char *p;
   int port = strtol(argv[2], &p, 10);

   // Setting the server ip addres, the connection type (TCP) and the connection port
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(port);

   // Converting conection from text to binary
   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
      perror("inet_pton error");
      exit(1);
   }

   // Establishing the connection
   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
      perror("connect error");
      exit(1);
   }

   // Getting the socket name and port
   u_int servaddr_len = sizeof(servaddr);
   if (getsockname(sockfd, (struct sockaddr *) &servaddr, &servaddr_len) == -1)
      perror("getsockname");
   else {
       // Finding local IP and local Port
      char connectionIP[16];
      
      inet_ntop(AF_INET, &servaddr.sin_addr, connectionIP, sizeof(connectionIP));
	   printf("Local ip address: %s, Local port: %u\n", connectionIP, ntohs(servaddr.sin_port));
   }
	
   // Receiving the messages from server, buffering and printing it
   while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      if (fputs(recvline, stdout) == EOF) {
         perror("fputs error");
         exit(1);
      }
   }

   if (n < 0) {
      perror("read error");
      exit(1);
   }

   exit(0);
}