#define VOTE_REQ 0xA9 /* 投票请求 */
#define INQY_REQ 0xAA /* 查询请求 */
#define INQY_RES 0xA8 /* 查询应答 */
#define HSIZE sizeof (struct vote) /* 投票协议头部大小 */
#define IDSIZE 1	/* 球队ID大小 */
#define VISIZE sizeof(struct voteinfo)/*投票信息大小*/
#define MAXTEAM 32	/* 球队数目 */
#define BUFSIZE 1024

/*客户查询兼投票消息结构体*/
struct vote {
    unsigned char type : 2,/* 协议类型 */
                    prec : 6;/* 前导符101010 */
    unsigned short len;/* 协议数据长度 */
    unsigned char content[0];/*柔性数组存放N个应答消息voteinfo*/
}__attribute__((packed));

/*服务器应答消息结构体*/
struct voteinfo {
    unsigned char id;/* 球队ID */
    unsigned int num;/* 球队得票数 */
}__attribute__((packed));

/*
*	创建查询应答消息
*	@voteinfo，球队投票信息列表
*	@num，待查询球队ID的个数
*	返回值，创建消息的大小
*/
int reply_vote (struct voteinfo *votelist_query, int num);





