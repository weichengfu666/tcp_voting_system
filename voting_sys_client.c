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
unsigned char reqBuf [BUFSIZE];	/* 请求数据协议单元 */ 
unsigned char acpBuf[BUFSIZE];/* 接收数据协议单元 */ 
struct voteinfo votelist[MAXTEAM]; /* 球队投票信息数组 */
unsigned char getStr[BUFSIZE];/*读取屏幕信息数组*/
unsigned char teamid[MAXTEAM];	/* 球队ID数组 */

/*从屏幕读取队伍信息*/
int getTeamId(char getStr[], char teamid[])
{
    int idNum, j,vote_flag;
    printf("\n================================新一轮操作=====================================\n");
    fputs("\n投票（请输入：1）  查询（请输入：0）：\n",stdout);
    vote_flag = getc(stdin)-'0';
    fputs("\n每输入一个队伍ID号（1-32）后按回车，输入（ok）后请求开始：\n",stdout);
    for (idNum = 0; idNum < MAXTEAM; )
    {
        fgets(getStr, BUFSIZE, stdin);

        if (!strcmp(getStr, "ok\n"))
        {
            break;
        }
        if (1 <= atoi(getStr) && atoi(getStr) <= 32)
        {
            teamid[idNum++] = atoi(getStr);
        }
    }
    teamid[idNum] = '\0';
    if(vote_flag){
        printf("\n为您投票了%2d支球队！\n", idNum);
    }else{
        printf("\n为您查询了%2d支球队！\n", idNum);
    }

    for (j = 0; j < idNum; j++)
    {
       printf("teamId = %2d\n", teamid[j]);
    }
    
    return vote_flag;
}

/*
*	创建投票、查询消息
*	@id，待查询球队ID列表
*	@vote, 为真代表投票请求，为假代表查询请求
*	返回值，创建消息的大小
*/
int make_vote (unsigned char * teamid, int isvote)
{
    struct vote v; 
    unsigned char type; 
    int num;
    int index;
    num = strlen(teamid);	/* 获得球队个数 */ 
    if(isvote)/*判断请求类型存入type*/
        type = VOTE_REQ;/*真为投票请求*/
    else
        type = INQY_REQ;/*假为查询请求*/
    memset(reqBuf, 0, BUFSIZE);/*清零协议单元*/
    *(unsigned char *)&v = type; /* 设置前导符和协议类型 */
    v.len = HSIZE + IDSIZE * num ; 
    memcpy(reqBuf, &v, HSIZE);	/* 构造协议头部 */
    if (num) 
        memcpy(reqBuf + HSIZE, teamid, IDSIZE * num); /* 球队ID */
    if(isvote){
            printf("\n投票请求消息总长度：协议头部（3）+ 投票信息（%d）=%2d\n",v.len-3,v.len);
    }else{
            printf("\n查票请求消息总长度：协议头部（3）+ 查票信息（%d）=%2d\n",v.len-3,v.len);
    }

    for(index = 0; index < v.len;index++)
    {
        if(isvote){
            printf("投票请求消息第%2d字节内容： %2x\n",index+1,*(reqBuf+index));
        }else{
            printf("查票请求消息第%2d字节内容： %2x\n",index+1,*(reqBuf+index));
        }
    }
    return v.len;
}


