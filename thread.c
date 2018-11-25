#include "thread.h"
#include<sys/stat.h>
#include<dirent.h>
#define MAXARG 10
#define PIPE_ERROR "pipe failed!"
#define FORK_ERROR "fork failed!"
#define NO_EXIST "file not exist"
#define NAME_ERROR "name inpput error!"
#define OPEN_ERROR "the fd open error!"
void serls (int c)
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
                    char *buff={0};
                    buff = entry->d_name;
                    send(c,buff,strlen(buff),0);
                    //printf("%s ",entry->d_name);
                }

            }
        }
    }
    printf("\n");
    closedir(dir);
    }
void put(int c,char *name[])
{
    char ser[128]={0};

    recv(c,ser,127,0);
    if(strncmp(ser,"ok#",3)!=0)
    {
        printf("%s\n",ser);
        return ;
    }

    int size = 0;
    sscanf(ser+3,"%d",&size);
    printf("file is %s , size is %d \n",name[1],size);

    int fd=open(name[1],O_CREAT|O_WRONLY,0600);
    if(fd ==-1)
    {
        send(c,OPEN_ERROR,strlen(OPEN_ERROR),0);
        return ;
    }
    send(c,"ok",2,0);
    char data[512] = {0};
    int num = 0;
    int cur_size = 0;
    while((num = recv(c,data,512,0))>0)
    {
        write(fd,data,num);
        cur_size+=num;
        float f = cur_size * 100.0/size;
        printf("loading:%.2f%%\r",f);
        fflush(stdout);

        if(cur_size>=size)
        {
            break;
        }
    }
    printf("\n");
    close(fd);
    printf("input finished\n");
    return ;
}
void download(int c ,char *name[])
{
    if(name[1]==NULL)
    {
        send(c,NAME_ERROR,strlen(NAME_ERROR),0);
        return ;
    }
    int fd = open(name[1],O_RDONLY);
    printf("open file \n ");
    if(fd == -1)
    {
        send(c,NO_EXIST,strlen(NO_EXIST),0);
        return ;
    }
    int size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    char res[64]= {0};
    strcpy(res,"ok#");
    sprintf(res+strlen(res),"%d",size);
    send(c,res,strlen(res),0);
    char cli[32] ={0};
    if(recv(c,cli,31,0)<=0)
    {
        close(fd);
        return ;
    }
    if(strcmp(cli,"ok")!=0)
    {
        close(fd);
        return ;
    }
    char data[1024]={0};
    int num = 0;
    while((num = read(fd,data,1024))>0)
    {
        send(c,data,num,0);
    }
    close(fd);
    printf("finished\n");
}
void* work_thread(void * arg) 

{
    int c = (int)arg;
    while( 1 )
    {
        char buff[128] = {0};
        if ( recv(c,buff,127,0) <=0 )
        {
            break;
        }
        char * argv[MAXARG] = {0};
        char * p=NULL;
        char * stok = strtok_r(buff," ",&p);
        int i=0;
        while(stok!=NULL)
        {
            argv[i++]=stok;
            stok = strtok_r(NULL," ",&p);
        }
        printf("[serhost@]$ %s\n",buff);
        if(argv[0]==NULL)
        {
            continue;
        }
       else if(strncmp(argv[0],"down",4)==0)
        {
            download(c,argv);
        }
       else  if(strncmp(argv[0],"put",3)==0)
        {
            put(c,argv);
        }
        else if(strncmp(argv[0],"serls",5)==0)
        {
            serls(c);
        }

        else
        {
            int pipefd[2];
            if(pipe(pipefd)==-1)
            {
                send(c,PIPE_ERROR,strlen(PIPE_ERROR),0);
                continue;
            }
            pid_t pid = fork();
            if(pid==-1)
            {
                send(c,FORK_ERROR,strlen(FORK_ERROR),0);
                continue;
            }
            if(pid==0)
            {
                dup2(pipefd[1],1);
                dup2(pipefd[1],2);
                execvp(argv[0],argv);
                perror("execvp error");
                exit(0);
            }
            close(pipefd[1]);
            wait(NULL);
            char readbuff[1024] = {"ok#"};
            read(pipefd[0],readbuff,1020);
            send(c,readbuff,strlen(readbuff),0);
        }
    }
    close(c);
    printf("one client over\n");
    return ;
}

void thread_start(int c)
{
    pthread_t id;
    pthread_create(&id,NULL,work_thread,(void*)c);
}

