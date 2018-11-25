#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "thread.h"

#define  PORT       6000
#define  IP_STR     "192.168.43.218"
#define  LIS_MAX    5

int create_socket();
int main()
{
    int sockfd = create_socket();
    assert( sockfd != -1 );

    while( 1 )
    {
        struct sockaddr_in caddr;
        int len = sizeof(caddr);
        int c = accept(sockfd,(struct sockaddr*)&caddr,&len);
        if ( c < 0 )
        {
            return ;
        }
        printf("accept c=%d\n",c);
        thread_start(c);
    }
    exit(0);
}

int create_socket()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if ( sockfd == -1 )
    {
        return -1;
    }

    struct sockaddr_in saddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(PORT);
    saddr.sin_addr.s_addr = inet_addr(IP_STR);
    
    int res = bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if ( res == -1 )
    {
        return -1;
    }

    listen(sockfd,LIS_MAX);
    return sockfd;
}
