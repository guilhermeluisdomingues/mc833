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

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

// Code based on Unix Networking Programming, Vol 1. page 228
void str_cli(FILE *fp, int sockfd)
{
   int maxfdp1, stdineof;
   fd_set rset;
   char buf[MAXLINE];
   int n;

   stdineof = 0;
   FD_ZERO(&rset);
   
   for ( ; ; ) {
      if (stdineof == 0)
         FD_SET(fileno(fp), &rset);

      FD_SET(sockfd, &rset);
      maxfdp1 = max(fileno(fp), sockfd) + 1;

      if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
         perror("select error");

      if (FD_ISSET(sockfd, &rset)) {
         if ((n = read(sockfd, buf, MAXLINE)) == 0) {
            if (stdineof == 1)
               return;
            else
               perror("str_cli: server terminated prematurely");
         }

         write(fileno(stdout), buf, n);
      }

      if (FD_ISSET(fileno(fp), &rset)) {
         if ((n = read(fileno(fp), buf, MAXLINE)) == 0)
         {
            stdineof = 1;
            shutdown(sockfd, SHUT_WR);
            FD_CLR(fileno(fp), &rset);
            continue;
         }
         write(sockfd, buf, n);
      }
   }
}

int main(int argc, char **argv)
{
   int sockfd;
   struct sockaddr_in servaddr;
   char error[MAXLINE + 1];
   FILE *fp = stdin;

   // The second argument expected is the ipaddres for connection
   if (argc < 3)
   {
      strcpy(error, "uso: ");
      strcat(error, argv[0]);
      strcat(error, " <IPaddress>");
      strcat(error, " <Port>");
      perror(error);
      exit(1);
   }

   // Creating the socket
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket error");
      exit(1);
   }

   // converting the Port argument (argv[2]) from string to int
   char *p;
   int port = strtol(argv[2], &p, 10);

   // Setting the server ip addres, the connection type (TCP) and the connection port
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(port);

   // Converting conection from text to binary
   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
   {
      perror("inet_pton error");
      exit(1);
   }

   // Establishing the connection
   if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
   {
      perror("connect error");
      exit(1);
   }

   str_cli(fp, sockfd);

   close(sockfd);

   exit(0);
}