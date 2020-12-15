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

int main(int argc, char **argv)
{
   int listenfd, connfd;
   struct sockaddr_in servaddr;
   char buf[MAXLINE];
   time_t ticks;

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
   servaddr.sin_port = htons(0); // The 0 port is used to get a random available port in computer

   // Converting connection text to binary
   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
   {
      perror("bind");
      exit(1);
   }

   // Getting the socket name and port
   u_int servaddr_len = sizeof(servaddr);
   if (getsockname(listenfd, (struct sockaddr *)&servaddr, &servaddr_len) == -1)
      perror("getsockname");
   else
   {
      // Finding local IP and local Port
      char connectionIP[16];
      inet_ntop(AF_INET, &servaddr.sin_addr, connectionIP, sizeof(connectionIP));
      printf("Server connected to %s:%u\n", connectionIP, ntohs(servaddr.sin_port));
   }

   // listening for connections
   if (listen(listenfd, LISTENQ) == -1)
   {
      perror("listen");
      exit(1);
   }

   maxfd = listenfd;
   maxi = -1;

   for (i = 0; i < FD_SETSIZE; i++)
      client[i] = -1;

   FD_ZERO(&allset);
   FD_SET(listenfd, &allset);

   for ( ; ; ) {
      rset = allset;
      nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

      if (FD_ISSET(listenfd, &rset)) {
         clilen = sizeof(cliaddr);
         connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

         for (i = 0; i < FD_SETSIZE; i++){

            if (client[i] < 0) {
               client[i] = connfd; /* save descriptor */
               break;
            }

            if (i == FD_SETSIZE)
               perror("too many clients");

            FD_SET(connfd, &allset);

            if (connfd > maxfd) maxfd = connfd;

            if (i > maxi) maxi = i;

            if (--nready <= 0) continue;
         }

         for (i = 0; i <= maxi; i++){

            if ((sockfd = client[i]) < 0)
               continue;

            if (FD_ISSET(sockfd, &rset))
            {

               if ((n = read(sockfd, buf, MAXLINE)) == 0) {
                  close(sockfd);
                  FD_CLR(sockfd, &allset);
                  client[i] = -1;
               }
               else {
                  write(sockfd, buf, n);
                  printf("write server: %s", buf);
               }

               if (--nready <= 0)
                  break;
            }
         }
      }
   }

   return (0);
}