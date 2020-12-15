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
#define MAXLINE 4096

int main(int argc, char **argv)
{
   int listenfd, connfd;
   struct sockaddr_in servaddr;
   char buf[MAXLINE];

   int i, maxi, maxfd, sockfd;
   int nready, client[FD_SETSIZE];
   ssize_t n;
   fd_set rset, allset;
   socklen_t clilen;
   struct sockaddr_in cliaddr;


   // Creating the socket in the text form
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      perror("socket");
      exit(1);
   }

   // Setting connection type (TCP), IP address and port.
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
   servaddr.sin_port = htons(1234);

   // Converting connection text to binary
   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }

   // Getting the socket name and port
   u_int servaddr_len = sizeof(servaddr);
   if (getsockname(listenfd, (struct sockaddr *)&servaddr, &servaddr_len) == -1)
      perror("getsockname");
   else {
      // Finding local IP and local Port
      char connectionIP[16];
      inet_ntop(AF_INET, &servaddr.sin_addr, connectionIP, sizeof(connectionIP));
      printf("Server connected to %s:%u\n", connectionIP, ntohs(servaddr.sin_port));
   }

   // listening for connections
   if (listen(listenfd, LISTENQ) == -1) {
      perror("listen");
      exit(1);
   }

   // This block send the messages for client
   for ( ; ; ) {
      // Accepting the connection with client
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
         perror("accept");
         exit(1);
      }

      while ((n = read(connfd, buf, MAXLINE)) > 0){
         if (write(connfd, buf, n) != n){
            perror("write");
            exit(1);
         }
      }

      // Closing connection with client
      close(connfd);
   }

   return (0);
}