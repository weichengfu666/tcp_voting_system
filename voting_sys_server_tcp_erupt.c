//利用进程和信号实现并发流式套接字服务器程序
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h> 
#include"voting_sys_server.h"

/*投票系统有关的变量*/
extern unsigned char rcvBuf [BUFSIZE];	/* 收数据协议单元 */ 
extern unsigned char returnBuf [BUFSIZE];/* 返回数据协议单元 */ 
extern struct voteinfo votelist[MAXTEAM]; /* 球队票数信息数组 */
extern struct voteinfo votelist_query[MAXTEAM];/* 球队查票信息数组 */

/*错误处理函数*/
static void bail(const char *on_what)
{
    fputs(strerror(errno), stderr);
    fputs(": ", stderr);
    fputs(on_what, stderr);
    fputc('\n', stderr);
    exit(1);
}


/*
* 设置子进程退出信号处理程序
*/
static void sigchld_handler(int signo){
pid_t pid;
int status;
char msg[] = "SIGCHLD caught\n";

write(STDOUT_FILENO, msg, sizeof(msg));

/* 等待已退出的所有子进程 */
do {
pid = waitpid (-1, &status, WNOHANG);
} while (pid > 0);
}


int main(int argc, char *argv[]) 
{
/*tcp连接有关的变量*/
    int sockfd;	/* 服务器套接字 */
    int new_fd;	/* 临时套接字 */
    struct sockaddr_in server_addr; /* 服务器监听地址 */
    struct sockaddr_in client_addr;	/* 客户端地址 */
    socklen_t sin_size;
    int portnumber;
    int z;

/*并发服务有关的变量*/
    pid_t pid;	/* 子进程pid */
    struct sigaction child_action;

/*投票系统有关的变量*/
    int idsum;//队伍数量
    int teamId;//队伍编号
    int index;
    int returnBuf_len = 0;/*发送缓存长度*/

/*初始化votelist的id*/
    for(index = 0; index < MAXTEAM; index ++)
    {
        votelist[index].id = index + 1;
    }

/* 设置SIGCHLD信号处理函数 */
    memset(&child_action, 0, sizeof(child_action));
    child_action.sa_flags |= SA_RESTART;
    child_action.sa_handler = sigchld_handler;
    if (sigaction(SIGCHLD, &child_action, NULL) == -1)
    perror("Failed to ignore SIGCHLD");

/*创建tcp监听套接字*/ 
    if(argc != 2){
        fprintf(stderr, "Usage: %s portnumber\a\n",
        argv[0]);
        exit(1); 
    }
    if((portnumber = atoi(argv[1]))<0){
        fprintf(stderr, "Usage: %s portnumber\a\n",
        argv[0]);
        exit(1);
    }
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket error: %s\a\n",
        strerror(errno));
        exit(1);
    }
    /* 构造服务器IP地址 */
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_addr.sin_port = htons(portnumber);
    /* 绑定服务器到指定地址和端口 */
    if ((bind(sockfd, (struct sockaddr *)(&server_addr), sizeof server_addr)) == -1) {
        fprintf(stderr, "Bind error: %s\a\n",
        strerror(errno));
        exit(1);
    }
    /* 监听 */
    if (listen(sockfd, 128) == -1) {
    fprintf(stderr, "Listen error: %s\n\a",
    strerror(errno));
    exit(1);
    }


/* 服务器端主循环 */
    for (;;) 
    {
        /*创建tcp连接套接字*/
        sin_size=sizeof (struct sockaddr_in);
        printf("Server is waiting for acceptance of new "
        "client...\n");
        /* 接收客户端连接并创建连接套接字 */
        if ((new_fd = accept(sockfd, (struct sockaddr
        *)(&client_addr), &sin_size)) == -1) {
            fprintf(stderr, "Accept error: %s\a\n",
            strerror(errno));
            exit(1);
        }
        fprintf(stdout, "Server got connection from %s\n",
        inet_ntoa(client_addr.sin_addr));

        /* 创建子进程 */
        pid = fork();
        switch (pid) 
        { 
            case -1:
                perror("fork failed");
                exit(1);
            case 0:	/* 子进程 */
                puts("Entering the child\n");
                /* 处理客户端请求循环 */
                for (;;) 
                {
                    z  = read(new_fd,rcvBuf,sizeof(rcvBuf));
                    if (z < 0)
                        bail("read()");
                    if (z == 0)/* 客户端关闭连接 */
                        break;
                    rcvBuf[z] = 0;/*添加NULL字符到接收到的请求字符串*/ 
                    /*打印接收到的信息*/
                    idsum = *((short *)(rcvBuf+1))-3;//计算收的了几个队伍请求
                    printf("\n===========================================新一轮操作==========================================\n\n");
                    if(VOTE_REQ==*(rcvBuf)){
                        printf("\n收到%2d次投票！\n",idsum);
                    }else{
                        printf("\n收到%2d次查票！\n",idsum);
                    }

                    for(index = 0; index < idsum+3; index++)
                    {
                        if(VOTE_REQ==*(rcvBuf)){
                            printf("投票请求消息第%2d字节内容： %x\n",index+1,*(rcvBuf+index));
                        }else{
                            printf("查票请求消息第%2d字节内容： %x\n",index+1,*(rcvBuf+index));
                        }
                    }
                    /*更改票数、打印队伍得票情况*/
                    if(VOTE_REQ==*(rcvBuf))
                    {   printf("\n投票信息如下：\n");
                        for(index = 0; index < idsum; index++)
                        {
                            teamId = *(rcvBuf+3+index);
                            votelist[teamId-1].num++;//投票增加票数
                            printf("teamId = %2d 增加1票，总得票数%2u\n", teamId,votelist[teamId-1].num);
                        }
                    }else{
                        printf("\n查票信息如下：\n");
                        for(index = 0; index < idsum; index++)
                        {
                            teamId = *(rcvBuf+3+index);//查询不增加票数
                            printf("teamId = %2d ，总得票数%2u\n", teamId,votelist[teamId-1].num);
                            votelist_query[index] = votelist[teamId-1];
                        }
                    }
                    /*创建应答消息*/
                    if(VOTE_REQ==*(rcvBuf)){/*投票返回*/
                        z = write(new_fd,"voting succeed!",strlen("voting succeed!"));
                        printf("\n投票成功\n");
                        if(z<0)
                            {
                                bail("write()");
                            } 
                    }else{/*查票返回*/
                        returnBuf_len = reply_vote (votelist_query, idsum);
                        printf("returnBuf_len = %d\n",returnBuf_len);
                        for(index = 0; index < returnBuf_len; index++)
                        {
                            printf("查票应答消息第%2d字节内容： %x\n",index,*(returnBuf+index));
                        }
                        z = write(new_fd,returnBuf,returnBuf_len);
                        printf("\n查票成功\n");
                        if(z<0)
                            {
                                bail("write()");
                            }
                        }
                }
                /* 结束此客户端处理后关闭连接套接字 */
                printf("Child process: %d exits.\n", getpid());
                close(new_fd);
                exit(0);

            default:
            /*
            * 在父进程中，只需要简单关闭对应的
            * 重复套接字文件描述符
            */
            puts ("this is the parent\n");
            close (new_fd);
        }
    }
}
