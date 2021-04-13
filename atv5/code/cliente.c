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
#include <sys/ioctl.h>
#include <sys/select.h>

#include "helpers.h"

#define MAXLINE 4096

int max(int a, int b)
{
  if (b > a)
  {
    return b;
  }
  return a;
}

int main(int argc, char **argv)
{
  int sockfd, is_done_reading = 0, stdin_fd = STDIN_FILENO, n;
  char buf[MAXLINE + 1];
  char error[MAXLINE + 1];
  struct sockaddr_in servaddr, peer_addr;
  unsigned int len, peer_len;
  fd_set rset;

  peer_len = sizeof(peer_addr);

  if (argc != 3)
  {
    strcpy(error, "uso: ");
    strcat(error, argv[0]);
    strcat(error, " <IPaddress> <PORT>");
    perror(error);
    exit(1);
  }

  // Initialize socket data
  sockfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(parse_port(argv[2]));
  len = sizeof(servaddr);
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
  {
    perror("inet_pton error");
    exit(1);
  }

  Connect(sockfd, &servaddr, len);

  // Get connection info from socket
  if (getsockname(sockfd, (struct sockaddr *)&servaddr, &len) == -1)
    perror("getsockname");
  if (getpeername(sockfd, (struct sockaddr *)&peer_addr, &peer_len) == -1)
    perror("getsockname");

  while (1)
  {
    FD_ZERO(&rset);
    if (!is_done_reading)
    {
      FD_SET(stdin_fd, &rset);
    }
    FD_SET(sockfd, &rset);
    select(max(sockfd, stdin_fd) + 1, &rset, NULL, NULL, NULL);

    if (FD_ISSET(sockfd, &rset))
    {
      if ((n = Readline(sockfd, buf, MAXLINE)) == 0)
      {
        if (is_done_reading)
          break;
        else
          perror("Client ended prematurely");
      }
      fputs("Jogadores em espera: ", stdout);
      fputs(buf, stdout);
      fputs("\n", stdout);
    }

    if (FD_ISSET(stdin_fd, &rset))
    {
      if ((n = Readline(stdin_fd, buf, MAXLINE)) == 0)
      {
        is_done_reading = 1;
        shutdown(sockfd, SHUT_WR);
        FD_CLR(stdin_fd, &rset);
        continue;
      }
      write(sockfd, buf, n);
      fputs(buf, stdout);
    }
  }

  close(sockfd);
  exit(0);
}
