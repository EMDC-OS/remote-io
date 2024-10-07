#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#define   SA      struct sockaddr
#define   PORT    8000

//#define FILENAME "received_image.yuyv"

#define BUFSIZE 1024

int main() 
{ 
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli;
    char buffer[BUFSIZE] = {0}; 
    char filename[64];  
    struct timeval start, end;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    { 
        perror("Socket creation error"); 
        exit(0);
    } 


    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr("115.145.179.144");

    //start
    gettimeofday(&start, NULL); 

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) 
    { 
        perror("connection with the server failed...\n"); 
        exit(0); 
    } 
    
    snprintf(filename, sizeof(filename), "%ld_%ld.yuyv", start.tv_sec, start.tv_usec);
    FILE *file = fopen(filename, "wb"); 
    if (!file) 
    { 
        perror("File opening failed"); 
        close(sockfd); 
        return -1; 
    } 
    
    int bytes; 
    while ((bytes = read(sockfd, buffer, BUFSIZE)) > 0) 
    {
        fwrite(buffer, 1, bytes, file); 
    } 

    //end
    gettimeofday(&end, NULL); 

    fclose(file); 
    close(sockfd); 
    
    double delay = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; 
    printf("Total delay: %.2f ms\n", delay); 

    return 0; 
}