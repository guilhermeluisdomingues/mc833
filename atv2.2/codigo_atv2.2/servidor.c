#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 4096
#define NUM_OF_COMMANDS 4

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

int Bind(int listenfd, struct sockaddr_in *servaddr, unsigned int len) {
   int result = bind(listenfd, (struct sockaddr *)servaddr, len);

   if (result < 0) {
      perror("bind");
      exit(1);
   }

   return result;
}

int Listen(int listenfd) {
   int result = listen(listenfd, LISTENQ);
   if (result < 0) {
      perror("accept");
      exit(1);
   }

   return result;
}

int Accept(int listenfd, struct sockaddr_in *peer_addr, unsigned int *peer_len) {
   int result = accept(listenfd, (struct sockaddr *)peer_addr, peer_len);
   if (result < 0) {
      perror("accept");
      exit(1);
   }
   return result;
}

int Fork() {
   pid_t forked = fork();
   if (forked < 0){
      perror("fork");
      exit(1);
   }
   return forked;
}

void logDateTime(FILE *file, char* status) {
   time_t ticks;
   ticks = time(NULL);
   fprintf(file, "%s at %.24s\r\n", status, ctime(&ticks));
}

void closeConnectionAndLog(FILE *file, int connfd, struct sockaddr_in peeraddr) {

   char connectionIP[16];
   inet_ntop(AF_INET, &peeraddr.sin_addr, connectionIP, sizeof(connectionIP));
   printf("Client %s:%u disconnected\n", connectionIP, ntohs(peeraddr.sin_port));

   logDateTime(file, "disconnected");
   close(connfd);
   exit(0);
}

int main (int argc, char **argv) {
   char   buf[MAXDATASIZE];
   char   error[MAXLINE + 1];
   char   recvline[MAXLINE + 1];
   
   int    listenfd, connfd = 0, n;
   struct sockaddr_in servaddr, peeraddr;
   
   u_int peerlen = sizeof(peeraddr);
   pid_t pid;
   FILE *file;

   if (argc < 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   listenfd = Socket(AF_INET, SOCK_STREAM, 0);

   // Setting connection type (TCP), IP address and port.
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

   char *p;
   int port = strtol(argv[1], &p, 10);
   servaddr.sin_port = htons(port);   // Getting the port from execution

   Bind(listenfd, &servaddr, sizeof(servaddr));

   Listen(listenfd);

   file = fopen("./server_log.out", "w");
   
   char commands[NUM_OF_COMMANDS][MAXDATASIZE] = {
      "pwd",
      "ls",
      "ps",
      "exit"
   };

   // This block send the messages for client
   for (;;) {
      connfd = Accept(listenfd, &peeraddr, &peerlen);

      logDateTime(file, "connected");

      pid = Fork();
      if (pid == 0) {
         close(listenfd);

         for (int i = 0; i < NUM_OF_COMMANDS; i++) {
            snprintf(buf, sizeof(buf), "%s", commands[i]);
            write(connfd, buf, strlen(buf));

            n = read(connfd, recvline, MAXLINE);
            recvline[n] = 0;

            if (strcmp(recvline, "exit") == 0) {
               closeConnectionAndLog(file, connfd, peeraddr);
            }

            if (n < 0) {
               perror("buffer error");
               exit(1);
            }
            fprintf(file, "command: %s\n", commands[i]);
            fprintf(file, "%s", recvline);
            fprintf(file, "\n");

            if (fputs(recvline, stdout) == EOF) {
               perror("fputs error");
               exit(1);
            }
         }

         closeConnectionAndLog(file, connfd, peeraddr);
      }
      close(connfd);
   }
   fclose(file);
   return(0);
}