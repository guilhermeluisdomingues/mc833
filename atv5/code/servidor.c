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
#include <signal.h>
#include <sys/poll.h>

#include "helpers.h"

#define OPENMAX 10
#define MAX_PLAYERS 10
#define ID_SIZE 10

// Log messages received from the client in a nice format
void log_response(FILE *server_file, char client_addr[ADDR_SIZE], char recvline[MAXLINE + 1])
{
    char time_buffer[26];
    fprintf(stdout, "[%s] %s Respose from client: %s\n", get_current_time(time_buffer), client_addr, recvline);
    fprintf(server_file, "[%s] %s Respose from client: %s\n", get_current_time(time_buffer), client_addr, recvline);
}

// Close a client connection and log it
void close_client_connection(struct pollfd client, char client_addr[ADDR_SIZE], FILE *server_file)
{
    char time_buffer[26];
    close(client.fd);
    client.fd = -1;
    fprintf(server_file, "[%s] Ended connection with: %s\n", get_current_time(time_buffer), client_addr);
    fprintf(stdout, "[%s] Ended connection with: %s\n", get_current_time(time_buffer), client_addr);
}

int main(int argc, char **argv)
{
    int listenfd, connfd, n, nready, max_idx, playersWaiting = 0;
    struct sockaddr_in servaddr, peer_addr_in;
    char recvline[MAXLINE + 1];
    unsigned int len, peer_len;
    char error[MAXLINE + 1];
    FILE *server_file;
    char time_buffer[26];
    struct pollfd clients[OPENMAX], waitingForPlay[OPENMAX];
    char clients_addr[OPENMAX][ADDR_SIZE], waitingForPlay_addr[OPENMAX][ADDR_SIZE];

    if (argc != 2)
    {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, "<Port>");
        perror(error);
        exit(1);
    }

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Initialize socket data
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(parse_port(argv[1]));
    len = sizeof(struct sockaddr);

    Bind(listenfd, &servaddr, len);
    Listen(listenfd);

    // First entry is dedicated to the listen socket
    clients[0].fd = listenfd;
    clients[0].events = POLLRDNORM;

    // Initialize all remaining sockets with a negative number
    for (int i = 1; i < OPENMAX; i++)
    {
        clients[i].fd = -1;
    }

    max_idx = 0;

    server_file = fopen("./server_log.out", "w");

    for (;;)
    {
        // poll for events
        nready = poll(clients, max_idx + 1, -1);

        // If the listen socket has a new event
        if (clients[0].revents & POLLRDNORM)
        {
            peer_len = sizeof(peer_addr_in);
            connfd = Accept(listenfd, &peer_addr_in, &peer_len);

            // Find a suitable client for the new connection
            int i;
            for (i = 1; i < OPENMAX; i++)
            {
                if (clients[i].fd < 0)
                {
                    clients[i].fd = connfd;
                    break;
                }
            }
            if (i == OPENMAX)
                perror("Too many clients accepted!");
            if (i > max_idx)
                max_idx = i;

            clients[i].events = POLLRDNORM;

            // Log the new connection
            if (getpeername(connfd, (struct sockaddr *)&peer_addr_in, &peer_len) == -1)
                perror("getpeername");
            
            snprintf(clients_addr[i], sizeof(clients_addr[i]), "%s:%d", inet_ntoa(peer_addr_in.sin_addr), ntohs(peer_addr_in.sin_port));
            fprintf(server_file, "[%s] New connection with: %s\n", get_current_time(time_buffer), clients_addr[i]);
            printf("[%s] New connection with: %s\n", get_current_time(time_buffer), clients_addr[i]);
            
            // Saving waiting players
            
            snprintf(waitingForPlay_addr[playersWaiting++], sizeof(waitingForPlay_addr[i]), "%s:%d", inet_ntoa(peer_addr_in.sin_addr), ntohs(peer_addr_in.sin_port));
            waitingForPlay[playersWaiting] = clients[i];

            for (int j = 0; j < playersWaiting; j++) {
                printf("Waiting: %s \n", waitingForPlay_addr[j]);           
            }

            // Update the number of events available
            if (--nready <= 0)
                continue;
        }


        for (int i = 1; i < OPENMAX; i++)
        {
            int sockfd = clients[i].fd;

            for (int j = 0; j<playersWaiting; j++) {
                write(clients[i].fd, waitingForPlay_addr[j], MAXLINE); // escrevendo para o cliente
            }

            if (sockfd < 0)
                continue;
            if (clients[i].revents & (POLLRDNORM | POLLERR))
            {
                if ((n = Readline(sockfd, recvline, MAXLINE)) < 0)
                {
                    if (errno == ECONNRESET){
                        close_client_connection(clients[i], clients_addr[i], server_file);
                        playersWaiting--;
                    }
                    else
                        perror("Unexpected error when reading from client");
                }
                else if (n == 0) {
                    close_client_connection(clients[i], clients_addr[i], server_file);
                    playersWaiting--;
                }
                else
                {
                    log_response(server_file, clients_addr[i], recvline);                    
                    write(sockfd, recvline, n); // escrevendo para o cliente
                }

                // Update the number of events available, early break if there are no more
                if (--nready <= 0)
                    break;
            }
        }
    }
    fclose(server_file);
    return (0);
}
