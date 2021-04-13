#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define LISTENQ 4
#define N_EXIT_STRINGS 5
#define EXIT_STRING_MAX_SIZE 10
#define MAXLINE 4096
#define ADDR_SIZE 20
/* ==============  Wrappers ==============  */

int Socket(int family, int type, int flags)
{
  int sockfd;
  if ((sockfd = socket(family, type, flags)) < 0)
  {
    perror("socket");
    exit(1);
  }
  else
  {
    return sockfd;
  }
}

int Listen(int listenfd)
{
  int result = listen(listenfd, LISTENQ);
  if (result == -1)
  {
    perror("accept");
    exit(1);
  }
  return result;
}

int Accept(int listenfd, struct sockaddr_in *peer_addr, unsigned int *peer_len)
{
  int result = accept(listenfd, (struct sockaddr *)peer_addr, peer_len);
  if (result < 0)
  {
    if (result != EINTR)
    {
      perror("accept");
      exit(1);
    }
  }
  return result;
}

int Fork()
{
  pid_t result = fork();
  if (result == -1)
  {
    perror("fork");
    exit(1);
  }
  return result;
}

int Bind(int listenfd, struct sockaddr_in *servaddr, unsigned int len)
{
  int result = bind(listenfd, (struct sockaddr *)servaddr, len);
  if (result == -1)
  {
    perror("bind");
    exit(1);
  }
  return result;
}

void Connect(int sockfd, struct sockaddr_in *servaddr, socklen_t len)
{
  if (connect(sockfd, (struct sockaddr *)servaddr, len) < 0)
  {
    perror("connect");
    exit(1);
  }
}

/* ==============  Helper functions ==============  */

// Function taken from: https://stackoverflow.com/questions/34035169/fastest-way-to-reverse-a-string-in-c
char *str_reverse(char *str)
{
  int len = strlen(str);
  char *p1 = str;
  char *p2 = str + len - 1;

  while (p1 < p2)
  {
    char tmp = *p1;
    *p1++ = *p2;
    *p2-- = tmp;
  }

  return str;
}

int is_exit_string(char *str)
{
  char exit_strings[N_EXIT_STRINGS][EXIT_STRING_MAX_SIZE] = {
      "exit",
      "sair",
      "bye",
      "quit"};

  for (int i = 0; i < N_EXIT_STRINGS; i++)
  {
    if (strcmp(str, exit_strings[i]) == 0 && str[0] != '\0')
    {
      return 1;
    }
  }
  return 0;
}

int read_from_socket(int socket, char *buf, size_t n_bytes)
{
  int n = read(socket, buf, n_bytes);
  if (n < 0)
  {
    perror("read error");
    exit(1);
  }
  buf[n] = 0;
  return n;
}

int read_line_from_socket(int socket, char *buf)
{
  char byte[2];
  int pos = 0, found_newline = 0;
  while (!found_newline)
  {
    int n = read(socket, byte, 1);
    if (n < 0)
    {
      perror("read error");
      exit(1);
    }

    // if (n == 0) {
    //   buf[pos] = '\0';
    //   found_newline = 1;
    // }

    buf[pos] = byte[0];
    if (byte[0] == '\n')
    {
      buf[pos + 1] = '\0';
      found_newline = 1;
    }
    pos++;
  }
  printf("Returning: %s\n", buf);
  return strlen(buf);
}

int parse_port(char *string_port)
{
  char *p;
  return strtol(string_port, &p, 10);
}

char *get_current_time(char time_buffer[26])
{
  // Vars to handle time and dates
  time_t timer;
  struct tm *tm_info;
  timer = time(NULL);
  tm_info = localtime(&timer);
  strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  return time_buffer;
}

static int read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];

static ssize_t
my_read(int fd, char *ptr)
{

  if (read_cnt <= 0)
  {
  again:
    if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
    {
      if (errno == EINTR)
        goto again;
      return (-1);
    }
    else if (read_cnt == 0)
      return (0);
    read_ptr = read_buf;
  }

  read_cnt--;
  *ptr = *read_ptr++;
  return (1);
}

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
  ssize_t n, rc;
  char c, *ptr;

  ptr = vptr;
  for (n = 1; n < maxlen; n++)
  {
    if ((rc = my_read(fd, &c)) == 1)
    {
      *ptr++ = c;
      if (c == '\n')
        break; /* newline is stored, like fgets() */
    }
    else if (rc == 0)
    {
      *ptr = 0;
      return (n - 1); /* EOF, n - 1 bytes were read */
    }
    else
      return (-1); /* error, errno set by read() */
  }

  *ptr = 0; /* null terminate like fgets() */
  return (n);
}

ssize_t
readlinebuf(void **vptrptr)
{
  if (read_cnt)
    *vptrptr = read_ptr;
  return (read_cnt);
}
/* end readline */

ssize_t
Readline(int fd, void *ptr, size_t maxlen)
{
  ssize_t n;

  if ((n = readline(fd, ptr, maxlen)) < 0)
    perror("readline error");
  return (n);
}
