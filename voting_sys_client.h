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


//从屏幕读取队伍信息
int getTeamId(char getStr[], char teamid[]);

/*
*	创建投票、查询消息
*	@id，待查询球队ID列表
*	@vote, 为真代表投票请求，为假代表查询请求
*	返回值，创建消息的大小
*/
int make_vote (unsigned char * teamid, int vote);



