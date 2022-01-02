#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<time.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include"voting_sys_server.h"

/*投票系统有关的变量*/
extern unsigned char rcvBuf [BUFSIZE];	/* 收数据协议单元 */ 
extern unsigned char returnBuf [BUFSIZE];/* 返回数据协议单元 */ 
extern struct voteinfo votelist[MAXTEAM]; /* 球队票数信息数组 */
extern struct voteinfo votelist_query[MAXTEAM];/* 球队查票信息数组 */

/*错误处理函数*/
static void bail(const char *on_what)
{
    fputs(strerror(errno),stderr);
    fputs(":",stderr);
    fputs(on_what,stderr);
    fputc('\n',stderr);
    exit(1);
}

int main(int argc, char *argv[])
{
/*tcp连接有关的变量*/
    int sockfd;//监听套接字
    int new_fd;//连接套接字
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t size;
    int portnumber;
    int z;


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

/*创建tcp监听套接字*/    
    if(argc != 2)
    {
        fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);
        exit(1);
    }
    if((portnumber=atoi(argv[1]))<0)   //atoi()  string to int
    {
        fprintf(stderr,"Usage:%s portnumber\a\n",argv[0]);
        exit(1);
    }
    if((sockfd=socket(PF_INET,SOCK_STREAM,0))==-1)
    {
        fprintf(stderr,"Socket error:%s\a\n",strerror(errno));
        exit(1);
    }

    /* 构造服务器IP地址 */
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(portnumber);

    /* 绑定服务器到指定地址和端口 */
    if((bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(server_addr))) == -1)
    {
        fprintf(stderr,"Bind error:%s\a\n",strerror(errno));
        exit(1);
    }
    if(listen(sockfd, 128) == -1)
    {
        fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
        exit(1);
    }
    printf("waiting for the client's request...\n");


/*主循环*/
    while(1)
    {
/*创建tcp连接套接字*/        
        size =sizeof(struct sockaddr_in);
        /* 接收客户端连接并创建连接套接字 */
        if((new_fd = accept(sockfd,(struct sockaddr *)(&client_addr),&size)) == -1)
        {
            fprintf(stderr,"Accept error:%s\a\n",strerror(errno));
            exit(1);
        }
        fprintf(stdout, "Server got connection from %s\n",inet_ntoa(client_addr.sin_addr));
        
        /* 处理客户端请求循环 */
        for(;;)
        {
           z  = read(new_fd,rcvBuf,sizeof(rcvBuf));
            if(z<0)
            {
                bail("read()");
            }
            if(0==z)/* 客户端关闭连接 */
            {
                close(new_fd);      
                break;
            }
            rcvBuf[z] = 0;
            

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
    }
}
