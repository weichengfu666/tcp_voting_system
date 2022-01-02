
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include"voting_sys_server.h"

/*和投票有关的变量*/
unsigned char rcvBuf [BUFSIZE];	/* 收数据协议单元 */ 
unsigned char returnBuf [BUFSIZE];/* 返回数据协议单元 */ 
struct voteinfo votelist[MAXTEAM]; /* 球队票数信息数组 */
struct voteinfo votelist_query[MAXTEAM];/* 球队查票信息数组 */

/*
*创建应答消息
*返回值，创建消息的大小
*/
int reply_vote (struct voteinfo *votelist_query, int num)
{
    struct vote v; 
    unsigned char type = INQY_RES; 
    memset(returnBuf, 0, BUFSIZE);/* 设置前导符和协议类型 */
    *(unsigned char *)&v = type;
    v.len = HSIZE + VISIZE * num;
    memcpy(returnBuf, &v, HSIZE);/* 构造协议头部 */
    memcpy(returnBuf + HSIZE, votelist_query, VISIZE * num); 
    return v.len;
}





