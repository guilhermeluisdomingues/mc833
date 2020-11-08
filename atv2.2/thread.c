#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>


void *myThreadFun(void *vargp) 
{ 
    printf("in thread \n"); 
    char input[100];
    scanf("%s", input);
    printf("%s", input);
    return NULL; 
} 

int main(int argc, char **argv) {
    pthread_t thread_id; 

    printf("Before Thread\n"); 
    pthread_create(&thread_id, NULL, myThreadFun, NULL);

    printf("main thread");

    pthread_join(thread_id, NULL);
    printf("After Thread\n"); 


    exit(0); 
}