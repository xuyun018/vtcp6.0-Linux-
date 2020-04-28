
#ifndef VTCP_H
#define VTCP_H
//---------------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#define VTCP_TIMER						(15)							//基本时钟
#define VTCP_TIMER_TIMES				(1000 / VTCP_TIMER)				//基本时钟每秒次数
#define VTCP_ASYNC_CACHE_COUNT			(32)							//异步数量
#define VTCP_ASYNC_CACHE_COUNT_MASK		(VTCP_ASYNC_CACHE_COUNT - 1)		//异步数量掩码
#define VTCP_PACKET_CACHE_COUNT			(128)							//包缓存
#define VTCP_PACKET_CACHE_COUNT_MASK	(VTCP_PACKET_CACHE_COUNT - 1)	//包缓存掩码
#define VTCP_PACKET_GROUP_COUNT			(3)

#define VTCP_CALC_SN_INTERVAL(s1,s2)	(int((unsigned int)(s1)-(unsigned int)(s2)))		//计算序列号间隔
#define VTCP_CALC_RTO(r)				(3 * (r) + VTCP_TIMER)			//计算包超时时间
#define VTCP_CALC_S2W(s,r)				(s * (r + 1) / 65536)			//计算速度到窗口
#define VTCP_CALC_S2B(s)				(s * (1000 ) / 65536)			//计算速度到带宽
#define VTCP_CALC_B2S(b)				(b * (65535) / 1000)			//计算带宽到速度

//包命令
enum VTCP_PKTCMD
{
	VTCP_PKTCMD_CONNECT = (0x0010),//连接
	VTCP_PKTCMD_CONNECT_ACK = (0x0011),//连接接收
	VTCP_PKTCMD_CONNECT_ACK_DELAY = (0x0012),//连接推迟(队列中)
	VTCP_PKTCMD_CONNECT_ACK_REFUSE = (0x0013),//连接拒绝(队列已满)
	VTCP_PKTCMD_DATA = (0x0020),//数据流
	VTCP_PKTCMD_DATA_ACK = (0x0021),//数据流应答
	VTCP_PKTCMD_RESET = (0x0030),//关闭通道
	VTCP_PKTCMD_RESET_ACK = (0x0031),//关闭通道应答
	VTCP_PKTCMD_SYNC = (0x0040),//同步控制包
	VTCP_PKTCMD_SYNC_ACK = (0x0041),//同步控制包

};


enum EVTcpPktMsk
{
	VTCP_PKTMSK_SEND = (0x0100),		//包已经发出
	VTCP_PKTMSK_SEND_REPEAT = (0x0200),		//包重复发出

};

//状态代码
enum vtcp_states
{
	VTCP_STATE_NULL = 0x00,//初始化
	VTCP_STATE_CREATED = 0x01,//创建
	VTCP_STATE_BINDED = 0x02,//绑定
	VTCP_STATE_LISTENED = 0x03,//监听
	VTCP_STATE_CONNECTING = 0x04,//连接进行
	VTCP_STATE_CONNECTED = 0x05,//连接完成
	VTCP_STATE_CONNRESET = 0x06,//连接被动断开(复位)
	VTCP_STATE_CLOSED = 0x07,//关闭

};

// SVTcpAsyncRecv
struct vtcp_buffer
{
	unsigned char *buffer;
	unsigned int length;
	unsigned int offset;
};

#pragma pack(push)
#pragma pack(1)

//4
struct vtcp_pkthdr
{
	//命令掩码
	uint16_t cmd;
	//SOCKET ID = socket 句柄
	uint16_t index;
};

#define VTCP_PACKET_DATA_SIZE (1024 - sizeof(struct vtcp_pkthdr) - 2 - 4 - 4)

//1024
struct vtcp_pktdata
{
	uint16_t ack_frequence;				//回包频率

	uint32_t tickcount;						//时间
	uint32_t sn;						//当前序号

	uint8_t data[VTCP_PACKET_DATA_SIZE];	//数据包数据
};

//14+1+32
struct vtcp_pktack
{
	uint32_t tickcount;						//时间
	uint32_t sn;						//当前序号

	//tcp_sn_recv_min
	uint16_t minimum;
	//tcp_sn_recv_max
	uint16_t maximum;
	//tcp_sn_recv_cur
	uint16_t current;

	//映像图字节大小
	uint8_t bitssize;
	//映像图
	uint8_t bits[VTCP_PACKET_CACHE_COUNT / 8];
};

//10+1+32
struct vtcp_pktsyncack
{
	uint32_t sn;

	//tcp_sn_recv_min
	uint16_t minimum;
	//tcp_sn_recv_max
	uint16_t maximum;
	//tcp_sn_recv_cur
	uint16_t current;
};

struct vtcp_pkt
{
	// 数据包头, 4 字节
	struct vtcp_pkthdr hdr;

	union
	{
		struct vtcp_pktdata data;
		struct vtcp_pktack ack;
		struct vtcp_pktsyncack synack;
	};
};

//包扩展(内存)
struct vtcp_pkt_ext
{
	//int64	speed;						//当前速度

	int cb;								//包大小（0：空包）
	int cbdata;							//包数据大小(数据包有效）
	int cboffset;						//包数据偏移(数据包有效,接收数据时使用)

	struct vtcp_pkt pkt;
};

#pragma pack(pop)

#define VTCP_SEND											1
#define VTCP_LOAD_SEND										2
#define VTCP_SENT											3
#define VTCP_RECV											4
#define VTCP_CONNECT										5
#define VTCP_ACCEPT											6
#define VTCP_LISTEN											7
#define VTCP_ADDRESSES_COMPARE								8
#define VTCP_ADDRESS_READ									9
#define VTCP_CANCEL											10
#define VTCP_TIMEOUT										11
// 申请空间
#define VTCP_REQUEST										12
// 释放空间
#define VTCP_RELEASE										13
#define VTCP_LOCK											14
// 以下是分支
#define VTCP_LOCK_SESSION									0
#define VTCP_LOCK_DOOR										1

typedef int (*t_vtcp_procedure)(void *parameter, unsigned int sid, unsigned int fd, unsigned char number, const unsigned char *address, unsigned int addresssize, void **packet, unsigned char *buffer, unsigned int bufferlength);

struct vtcp_queue
{
	unsigned char queue[sizeof(struct vtcp_buffer) * VTCP_ASYNC_CACHE_COUNT];

	unsigned int index;
	unsigned int count;
};

struct vtcp_door
{
	//

	unsigned char address[20];
};

struct vtcp_packet
{
	struct vtcp_pkt_ext packets[VTCP_PACKET_CACHE_COUNT];

	unsigned int count;
};

struct vtcp_session
{
	//标识(虚拟句柄)
	// 远程
	unsigned short index1;

	//状态
	unsigned char state;
	//标志
	unsigned char flags;

	unsigned int last_send;
	unsigned int last_recv;

	// 地址
	unsigned char address[20];

	// 发送数据列表
	struct vtcp_queue queue1;
	// 接收包
	struct vtcp_packet packet0;
	// 发送包
	struct vtcp_packet packet1;

	int	errorcode;							//内部错误代码

	//序号发生器
	unsigned int sn;

	//发送序号底端(被确认序号)
	unsigned int minimum1;
	//发送序号高端(最大许可序号)
	unsigned int maximum1;
	//发送序号当前(发送但未确认序号)
	unsigned int current1;
	//发送序号更新(序号)
	unsigned int update;

	//接收序号底端(确认序号但未传输给客户)
	unsigned int minimum0;
	//接收序号高端(最大许可序号)
	unsigned int maximum0;
	//接收序号当前(确认序号)
	unsigned int current0;

	//TCP-RTT测量
	unsigned int rtt;							//发包往返周期（RTT）
	unsigned int rtt_prev;						//发包往返周期（RTT）

	//TCP发包速度控制
	unsigned long long	send_data_speed_surplus;		//发包速度剩余（65536余）
	unsigned long long	send_data_speed;				//发包速度（65536倍）每毫秒
	unsigned long long	send_data_speed_prev;			//发包速度（65536倍）每毫秒
	unsigned long long	send_data_speed_change;		//发包速度变化量
	unsigned long long	send_data_speed_change_prev;	//发包速度变化量
	int send_data_speed_level;		//发包速度水平（0~15）

	// 重发个数
	unsigned int repeat;
	unsigned int send_count;
	// 发包拥塞窗口
	unsigned int cwnd_prev_prev;
	// 发包拥塞窗口
	unsigned int cwnd_prev;
	// 发包拥塞窗口
	unsigned int cwnd;

	// TCP回包速度控制
	// 接收数据包个数
	unsigned int recv_count;
	// 接收数据包回应频率
	unsigned int recv_ack_freq;

	// public://TCP带宽预测
	// 	int		m_tcp_recv_data_series_speed;		
	// 	int		m_tcp_recv_data_series_space;
	// 	int		m_tcp_recv_data_series_count;
	// 	int		m_tcp_recv_data_series_count_temp;
	// 	int		m_tcp_recv_data_series_time0;
	// 	int		m_tcp_recv_data_series_time1;
	// 	int		m_tcp_recv_data_series_time2;

	//TCP参数
	//逗留
	unsigned int linger;
	// 逗留时间
	unsigned int linger_timeout;
	//逗留(内部使用)
	unsigned int linger_timeout_tick;
	// 活动时间
	unsigned int keepalive;
	// 活动时间间隔
	unsigned int keepalive_internal;
	// 连接超时
	unsigned int connect_timeout;
	// 连接超时(内部使用)
	unsigned int connect_timeout_tick;

	//统计变量
	unsigned long long count_do_send_data;
	unsigned long long count_do_send_data_ack;
	unsigned long long count_do_send_data_ack_lost;
	unsigned long long count_do_send_data_repeat;
	unsigned long long count_do_send_sync;
	unsigned long long count_do_send_sync_ack;
	unsigned long long count_do_send_reset;
	unsigned long long count_do_send_reset_ack;

	//统计变量
	unsigned long long count_on_recv_data;
	unsigned long long count_on_recv_data_ack;
	unsigned long long count_on_recv_data_ack_lost;
	unsigned long long count_on_recv_sync;
	unsigned long long count_on_recv_sync_ack;
	unsigned long long count_on_recv_reset;
	unsigned long long count_on_recv_reset_ack;

	//统计速度
	unsigned long long count_recv;
	unsigned long long count_send;
	unsigned long long count_send_bytes;
};

struct vtcp
{
	// 最多 32 个监听, 实际上使用通常已经太多了
	struct vtcp_door doors[32];
	unsigned int door_count;

	struct vtcp_session *sessions;
	unsigned int count;

	void *parameter;

	t_vtcp_procedure p_procedure;

	unsigned int fd;
};

void vtcp_initialize(struct vtcp *pvtcp, void *parameter, t_vtcp_procedure p_procedure);
void vtcp_uninitialize(struct vtcp *pvtcp);

unsigned int vtcp_set_ranks(struct vtcp *pvtcp, unsigned int count);

struct vtcp_door *vtcp_search_door(struct vtcp *pvtcp, const unsigned char *address, unsigned int addresssize);

struct vtcp_door *vtcp_door_open(struct vtcp *pvtcp, void *pointer, const unsigned char *address, unsigned int addresssize);
int vtcp_door_close(struct vtcp *pvtcp, struct vtcp_door *pdoor);

#endif