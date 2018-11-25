#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<dirent.h>
#include<sys/stat.h>
#include <fcntl.h>
#define IP "192.168.43.218"
#define DOWN_ERROR "download error 0"
#define PIPE_ERROR "pipe failed!"
#define FORK_ERROR "fork failed!"
#define NO_EXIST "file not exist"
#define NAME_ERROR "name inpput error!"
void serls(int sockfd)
{
    char buff[128]={0};
    if( recv(sockfd,buff,127,0)<=0)
    {
        printf("recv error");
        }
   else
    {
        printf("%s\n",buff);
    }
    return ;
    }
void myls()
{
    DIR *dir = opendir(".");
    if(NULL == dir)
    {
        printf("no file\n");
        exit(1);
    }
    struct dirent *entry = NULL;
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)
        {
            continue;
        }
        else
        {
            struct stat st;
            lstat(entry->d_name,&st);
            if(S_ISDIR(st.st_mode))
            {
                printf("\033[1m\033[34m%s\033[0m  ",entry->d_name);
            }
            else if(S_ISREG(st.st_mode))
            {
                if(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
                {
                    printf("\033[1m\033[32m%s\033[0m ",entry->d_name);
                }
                else
                {
                    printf("%s ",entry->d_name);
                }

            }
        }
    }
    printf("\n");
    closedir(dir);
    }

void put(int sockfd,char *name[])
{
    if(name[1]==NULL)
    {
        send(sockfd,NAME_ERROR,strlen(NAME_ERROR),0);
        return;
        }
    int fd = open(name[1],O_RDONLY);
    printf("open file\n");
    if(fd==-1)
    {
        send(sockfd,NO_EXIST,strlen(NO_EXIST),0);
        return;
        }
    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    char res[64]={0};
    strcpy(res,"ok#");
    sprintf(res+strlen(res),"%d",size);
    send(sockfd,res,strlen(res),0);
    char ser[32]={0};
    if(recv(sockfd,ser,31,0)<0)
    {
        close(fd);
        return;
        }

    if(strcmp(ser,"ok")!=0)
    {
        close(fd);
        return;
        }
    char data[512]={0};
    int num = 0;
    while((num=read(fd,data,512))>0)
    {
        send(sockfd,data,num,0);
        }
    close(fd);
    printf("finished\n");
    }

void download(int sockfd ,char *name[])
{
    char res[128]= {0};
    recv(sockfd,res,127,0);
    if (strncmp(res,"ok#",3)!=0)
    {
        printf("%s\n",res);
        return ;
    }
    int size = 0;
    sscanf(res+3,"%d",&size);

    printf("file:%s,size:%d\n",name[1],size);

    int fd = open(name[1],O_CREAT|O_WRONLY,0600);
    if(fd == -1)
    {
        send(sockfd,"err",3,0);
        return ;
    }
    send (sockfd,"ok",2,0);
    char data[1024] = {0};
    
    int num = 0;
   int cur_size =0; 
    while((num=recv(sockfd,data,1024,0))>0)
    {
        write(fd,data,num);
        cur_size+=num;
        float f = (cur_size * 100.0) /size;
        printf("loading:%.2f%%\r",f);
        fflush(stdout);
        if(cur_size>=size)
        {
            break;
        }
    }
    printf("\n");
    close(fd);
    printf("download finished\n");
    return ;
}
int main()
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert( sockfd != -1 );

    struct sockaddr_in saddr,caddr;
    memset(&saddr,0,sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr(IP);

    int res = connect(sockfd,(struct sockaddr*)&saddr,sizeof(saddr));
    assert( res != -1 );

    while( 1 )
    {
        char buff[1024] = {0};
        printf("input:\n");
        fgets(buff,128,stdin);
        buff[strlen(buff)-1]='\0';
        char  tmp[1024] = {0};
        strcpy(tmp,buff);
        char* myargv[10] = {0};
        int i =0;
        char *s = strtok(tmp," ");
        while(s!=NULL)
        {
            myargv[i++]=s;
            s = strtok(NULL," ");

        }
        if ( myargv[0] == NULL )
        {
            continue;
        }
        if ( strncmp(myargv[0],"end",3) == 0 )
        {
            break;
        }
        else  if(strncmp(myargv[0],"down",4)==0)
        {
             send(sockfd,buff,strlen(buff),0);
             download(sockfd,myargv);
        } 
        else if(strncmp(myargv[0],"put",3)==0)
        {
            send(sockfd,buff,strlen(buff),0);
            put(sockfd,myargv);
            }
        else if(strncmp(myargv[0],"myls",3)==0)
        {
            myls();
            }
        else if(strncmp(myargv[0],"serls",5)==0)
        {
            serls(sockfd);
            }

         else {send(sockfd,buff,strlen(buff),0);
         memset(buff,0,128);
         recv(sockfd,buff,1024,0);
         printf("%s\n",buff);}
    }
    close(sockfd);
    exit(0);
}
