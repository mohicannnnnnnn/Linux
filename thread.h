#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include<sys/syscall.h>
void thread_start(int c);

void download(int c,char *name[]);

void* work_thread(void *arg);

