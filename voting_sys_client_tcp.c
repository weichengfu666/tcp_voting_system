    #include <stdlib.h>
    #include <stdio.h>
    #include <errno.h>
    #include <string.h>
    #include <netdb.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include"voting_sys_client.h"

/*和投票有关的变量*/    
    extern unsigned char reqBuf[BUFSIZE];	/* 请求数据协议单元 */ 
    extern unsigned char acpBuf[BUFSIZE];/* 接收数据协议单元 */ 
    extern unsigned char getStr[BUFSIZE];/*读取屏幕信息数组*/
    extern unsigned char teamid[MAXTEAM];	/* 球队ID数组 */
    extern struct voteinfo votelist[MAXTEAM]; /* 球队投票信息数组 */

/*出错处理函数*/
    static void bail(const char *on_what){
        fputs(strerror(errno), stderr);
        fputs(": ", stderr);
        fputs(on_what, stderr);
        fputc('\n', stderr);
        exit(1);
    }


/*主函数入口*/
int main (int argc, char *argv[]) {

/*和tcp连接有关的变量*/
    int sockfd;	
    struct sockaddr_in server_addr;	
    struct hostent *host;
    int portnumber;
    int nbytes;
    int z;

/*和投票有关的变量*/
int vote_flag = 10;/*投票标志位 1为投票*/
int idsum=0;/*选择队伍数量*/
int index;/*访问下标*/
int reqBuf_len;/*发送缓存长度*/	


/*建立tcp连接*/
    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname portnumber\a\n",
        argv[0]);
        exit(1);
    }
    if ((host = gethostbyname(argv[1])) == NULL) {
        fprintf(stderr, "Gethostname error\n");
        exit(1);
    }
    if ((portnumber = atoi(argv[2])) < 0) {
        fprintf(stderr, "Usage: %s hostname portnumber\a\n",
        argv[0]);
        exit(1);
    }
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Socket Error: %s\a\n",
        strerror(errno));
        exit(1);
    }
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portnumber);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    if (connect(sockfd, (struct sockaddr *)(&server_addr),
        sizeof server_addr) == -1) {
        fprintf(stderr, "Connect Error: %s\a\n",
        strerror(errno));
        exit(1);
    }
    printf("connected to server %s\n",
    inet_ntoa(server_addr.sin_addr));

/*主循环*/
    for (;;) 
    {

/*创建请求消息*/
vote_flag = getTeamId(getStr, teamid);
reqBuf_len = make_vote (teamid,vote_flag);


/*发送请求消息*/
        z = write(sockfd, reqBuf, reqBuf_len);
        if (z < 0)
        bail("write()");
        printf("\nclient has sent  to the sever...\n");

/*接收应答消息*/
        if ((nbytes = read(sockfd, acpBuf, sizeof acpBuf)) == -1) 
        {
            fprintf(stderr, "Read Error: %s\n",
            strerror(errno));
            exit(1);
        }
        if (nbytes == 0) { 
            printf("server has closed the socket.\n");
            printf("press any key to exit...\n");
            getchar();
            break;
        }
        acpBuf[nbytes] = '\0'; 
        printf("result from %s port %u :\n",
        inet_ntoa(server_addr.sin_addr),
        (unsigned)ntohs(server_addr.sin_port));
       
 /*打印接收的应答消息*/
        if(vote_flag)
        {
            printf("%s",acpBuf); //查票应答消息
        }else
        {//投票应答消息
            idsum = (*((short*)(acpBuf+1))-3)/5;//计算应答了几个队伍

            memcpy(votelist, acpBuf + HSIZE, VISIZE * idsum );//将接收缓存数据放入votelist
            for(index = 0; index < idsum; index++)
            {
                printf("teamId = %2d 总得票数%2d\n",votelist[index].id,votelist[index].num);//打印查询的投票结果
            }
        }
       
    }

/*关闭和服务器的连接*/
    close(sockfd);
    return 0;
}

