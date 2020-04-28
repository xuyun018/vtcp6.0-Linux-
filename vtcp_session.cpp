#include "vtcp_session.h"
//---------------------------------------------------------------------------
void vtcp_session_uninitialize(struct vtcp_session *psession);
//---------------------------------------------------------------------------
struct vtcp_session *vtcp_seek_session(struct vtcp *pvtcp, unsigned int index)
{
	struct vtcp_session *result = NULL;
	unsigned int i;

	if (index < pvtcp->count)
	{
		result = pvtcp->sessions;
		result += index;
	}
	return(result);
}

void vtcp_send_connect(struct vtcp *pvtcp, unsigned short index, const unsigned char *address, unsigned int addresssize)
{
	struct vtcp_pkt_ext ppe[1];

	ppe->cb = sizeof(struct vtcp_pkthdr);
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_CONNECT);
	// 自己的 index
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, index);

	// 发送数据包
	pvtcp->p_procedure(pvtcp->parameter, index, pvtcp->fd, VTCP_SEND, address, addresssize, NULL, (unsigned char *)&ppe->pkt, ppe->cb);
}
void vtcp_send_dataack(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int sn, unsigned int tickcount)
{
	struct vtcp_pkt_ext ppe[1];
	unsigned int bitssize;

	if (VTCP_CALC_SN_INTERVAL(sn, psession->current0 + 1) > 0)
	{
		bitssize = vtcp_packet_makebits(&psession->packet0, sn, psession->current0 + 1, ppe->pkt.ack.bits);
	}
	else
	{
		bitssize = 0;
	}

	ppe->cb = sizeof(struct vtcp_pkthdr) + sizeof(struct vtcp_pktack) - (VTCP_PACKET_CACHE_COUNT / 8) + bitssize;
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_DATA_ACK);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, psession->index1);

	vtcp_write4bytes((unsigned char *)&ppe->pkt.ack.tickcount, tickcount);
	vtcp_write4bytes((unsigned char *)&ppe->pkt.ack.sn, sn);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.ack.current, psession->current0 - sn);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.ack.minimum, psession->minimum0 - sn);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.ack.maximum, psession->maximum0 - sn);
	ppe->pkt.ack.bitssize = bitssize;

	// 发送数据包
	pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SEND, psession->address, sizeof(psession->address), NULL, (unsigned char *)&ppe->pkt, ppe->cb);
	psession->last_send = tickcount;

	psession->count_do_send_data_ack++;
}
void vtcp_send_sync(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int tickcount)
{
	struct vtcp_pkt_ext ppe[1];

	ppe->cb = sizeof(struct vtcp_pkthdr);
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_SYNC);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, psession->index1);

	// 发送数据包
	pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SEND, psession->address, sizeof(psession->address), NULL, (unsigned char *)&ppe->pkt, ppe->cb);
	psession->last_send = tickcount;

	psession->count_do_send_sync++;
}
void vtcp_send_syncack(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int tickcount)
{
	struct vtcp_pkt_ext ppe[1];

	ppe->cb = sizeof(struct vtcp_pkthdr) + sizeof(struct vtcp_pktsyncack);
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_SYNC_ACK);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, psession->index1);

	vtcp_write4bytes((unsigned char *)&ppe->pkt.synack.sn, psession->minimum0);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.synack.current, psession->current0 - psession->minimum0);
	ppe->pkt.synack.minimum = 0;
	vtcp_write2bytes((unsigned char *)&ppe->pkt.synack.maximum, psession->maximum0 - psession->minimum0);

	// 发送数据包
	pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SEND, psession->address, sizeof(psession->address), NULL, (unsigned char *)&ppe->pkt, ppe->cb);
	psession->last_send = tickcount;

	psession->count_do_send_sync_ack++;
}
void vtcp_send_reset(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int tickcount)
{
	struct vtcp_pkt_ext ppe[1];

	ppe->cb = sizeof(struct vtcp_pkthdr);
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_RESET);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, psession->index1);

	// 发送数据包
	pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SEND, psession->address, sizeof(psession->address), NULL, (unsigned char *)&ppe->pkt, ppe->cb);
	psession->last_send = tickcount;

	psession->count_do_send_reset++;
}
void vtcp_send_resetack(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int tickcount)
{
	struct vtcp_pkt_ext ppe[1];

	ppe->cb = sizeof(struct vtcp_pkthdr);
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_RESET_ACK);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, psession->index1);

	// 发送数据包
	pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SEND, psession->address, sizeof(psession->address), NULL, (unsigned char *)&ppe->pkt, ppe->cb);
	psession->last_send = tickcount;

	psession->count_do_send_reset_ack++;
}

int vtcp_queue_cancel(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, int errorcode)
{
	struct vtcp_buffer *pb;

	while (pb = vtcp_queue_getfirst(&psession->queue1))
	{
		// 通知
		pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_CANCEL, psession->address, sizeof(psession->address), NULL, pb->buffer, pb->length);

		vtcp_queue_skip(&psession->queue1);
	}
	return(0);
}

void vtcp_session_cancel(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid)
{
	//ERROR_CANCELLED
	vtcp_queue_cancel(pvtcp, psession, sid, VTCP_PKTCMD_RESET);
}

// 设计已经不同了, 废弃
int vtcp_session_close(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, int force, unsigned int tickcount)
{
	if (psession->state == VTCP_STATE_CONNECTED && psession->packet1.count && psession->linger && !force)
	{
		if (psession->linger_timeout_tick == 0)
		{
			psession->linger_timeout_tick = tickcount;

			return VTCP_PKTCMD_RESET;
		}
		else if (tickcount - psession->linger_timeout_tick < psession->linger_timeout)
		{
			return VTCP_PKTCMD_RESET;
		}
	}

	if (psession->state == VTCP_STATE_CONNECTED && !force)
	{
		psession->state = VTCP_STATE_CONNRESET;

		vtcp_send_reset(pvtcp, psession, sid, tickcount);

		return VTCP_PKTCMD_RESET;
	}

	psession->state = VTCP_STATE_CLOSED;

	vtcp_queue_cancel(pvtcp, psession, sid, VTCP_PKTCMD_RESET);

	return(0);
}

// 把缓冲中的数据加入到发送队列
unsigned int vtcp_load_send_buffers(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid)
{
	struct vtcp_buffer *pb;
	struct vtcp_pkt_ext *ppe;
	unsigned int l;
	unsigned int result = 0;

	while (pb = vtcp_queue_getfirst(&psession->queue1))
	{
		while (ppe = vtcp_packet_alloc(&psession->packet1, psession->sn + 1))
		{
			l = pb->length - pb->offset;
			l = l < VTCP_PACKET_DATA_SIZE ? l : VTCP_PACKET_DATA_SIZE;

			ppe->cb = sizeof(struct vtcp_pkt) - VTCP_PACKET_DATA_SIZE + l;
			ppe->cbdata = l;
			ppe->cboffset = 0;

			vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, VTCP_PKTCMD_DATA);
			vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, psession->index1);

			vtcp_write2bytes((unsigned char *)&ppe->pkt.data.ack_frequence, VTCP_PACKET_GROUP_COUNT);
			ppe->pkt.data.tickcount = 0;
			vtcp_write4bytes((unsigned char *)&ppe->pkt.data.sn, ++psession->sn);

			memcpy(ppe->pkt.data.data, pb->buffer + pb->offset, l);

			psession->count_send_bytes += l;

			if ((pb->offset += l) < pb->length)
			{
				continue;
			}

			result += pb->offset;

			//
			pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_LOAD_SEND, psession->address, sizeof(psession->address), NULL, pb->buffer, pb->offset);

			vtcp_queue_skip(&psession->queue1);

			break;
		}

		if (ppe == NULL)
		{
			break;
		}
	}
	return(result);
}
// group:最小包组，only:仅检查超时包
void vtcp_send_buffers(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int group, int onlyTimeout, unsigned int tickcount)
{
	struct vtcp_pkt_ext *ppes[VTCP_PACKET_CACHE_COUNT];
	struct vtcp_pkt_ext *ppe;
	unsigned int current;
	unsigned int count = 0;
	unsigned int count_group;
	unsigned int i;

	// 有需要发送的数据包
	if (psession->packet1.count)
	{
		current = psession->minimum1;

		count_group = psession->send_data_speed_surplus >> 16;

		// 所有数据都发送完毕了
		// 一开始的时候, minimum + VTCP_PACKET_CACHE_COUNT = maximum
		if (psession->minimum1 == psession->maximum1)
		{
			vtcp_send_sync(pvtcp, psession, sid, tickcount);
		}
		else
		{
			if (psession->current1 == psession->minimum1)
			{
				count_group = VTCP_PACKET_GROUP_COUNT;
			}
			// 粒度限制, coung_group < group 的时候不处理
			if (count_group >= group)
			{
				// 重复发送
				while (count < count_group && (int)(current + 1 - psession->current1) <= 0)
				{
					ppe = vtcp_packet_get(&psession->packet1, current + 1);
					if (ppe)
					{
						if (tickcount - ppe->pkt.data.tickcount > VTCP_CALC_RTO(psession->rtt))
						{
							ppes[count++] = ppe;

							vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, vtcp_read2bytes((unsigned char *)&ppe->pkt.hdr.cmd) | VTCP_PKTMSK_SEND_REPEAT);
						}
					}
					else
					{
						//已确认（有可能发生）
					}

					current++;
				}

				if (count)
				{
					psession->count_do_send_data_repeat += count;
					psession->repeat += count;
				}

				while (count < count_group && !onlyTimeout)
				{
					if (VTCP_CALC_SN_INTERVAL(psession->current1 + 1, psession->maximum1) > 0)
					{
						break;	//限制(速度)
					}

					if (VTCP_CALC_SN_INTERVAL(psession->current1 + 1, psession->sn) > 0)
					{
						break;	//限制(缓存)
					}

					ppe = vtcp_packet_get(&psession->packet1, psession->current1 + 1);
					if (ppe)
					{
						ppes[count++] = ppe;

						vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, vtcp_read2bytes((unsigned char *)&ppe->pkt.hdr.cmd) | VTCP_PKTMSK_SEND);

						psession->current1++;
					}
					else
					{
						break; //无包发送的情况
					}
				}

				if (count)
				{
					psession->count_do_send_data += count;
					psession->send_count += count;
				}

				for (i = 0; i < count; i++)
				{
					vtcp_write2bytes((unsigned char *)&ppes[i]->pkt.data.ack_frequence, (i == count - 1) ? 1 : VTCP_PACKET_GROUP_COUNT);
					vtcp_write4bytes((unsigned char *)&ppes[i]->pkt.data.tickcount, tickcount);

					pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SEND, psession->address, sizeof(psession->address), NULL, (unsigned char *)&ppes[i]->pkt, ppes[i]->cb);
				}
				psession->last_send = tickcount;

				if (count)
				{
					psession->send_data_speed_surplus -= count << 16;
				}
			}
		}
	}
}
int vtcp_send(struct vtcp *pvtcp, unsigned int sid, const unsigned char *buffer, unsigned int length, unsigned int tickcount)
{
	struct vtcp_session *psession;
	struct vtcp_buffer *pb = NULL;

	pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 1);
	psession = vtcp_seek_session(pvtcp, sid);
	if (psession)
	{
		pb = vtcp_queue_alloc(&psession->queue1);
		if (pb)
		{
			pb->buffer = (unsigned char *)buffer;
			pb->length = length;
			pb->offset = 0;

			vtcp_load_send_buffers(pvtcp, psession, sid);

			if (psession->state == VTCP_STATE_CONNECTED)
			{
				if (psession->minimum1 == psession->current1)
				{
					vtcp_send_buffers(pvtcp, psession, sid, 1, 0, tickcount);
				}
			}

			psession->count_send++;
		}
	}
	pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 0);

	return(pb != NULL);
}

void vtcp_load_recv_buffers(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid)
{
	struct vtcp_pkt_ext *ppe;

	while (ppe = vtcp_packet_get(&psession->packet0, psession->minimum0 + 1))
	{
		pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_RECV, psession->address, sizeof(psession->address), NULL, (unsigned char *)ppe->pkt.data.data, ppe->cbdata - ppe->cboffset);

		vtcp_packet_free(&psession->packet0, psession->minimum0 + 1);

		psession->minimum0++;
		psession->maximum0++;
	}
}

int vtcp_update_speed(struct vtcp_session *psession)
{
	int sendvar = VTCP_CALC_SN_INTERVAL(psession->current1, psession->minimum1);
	
	int sending = (int)psession->packet1.count - VTCP_CALC_SN_INTERVAL(psession->sn, psession->current1);

	psession->send_data_speed_change_prev = psession->send_data_speed_change;

	if ((sendvar > sending) || psession->repeat)
	{
		switch (psession->send_data_speed_level)
		{
		case 0: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x2) - 1;	break;
		case 1: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x3) - 1;	break;
		case 2: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x4) - 1;	break;
		case 3: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x5) - 1;	break;
		case 4: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x6) - 1;	break;
		case 5: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x7) - 1;	break;
		case 6: psession->send_data_speed_change = 0 - (psession->send_data_speed >> 0x8) - 1;	break;

		default:
			break;
		}

		if (psession->repeat > 2) //丢包处理
		{
			psession->send_data_speed_change = psession->send_data_speed_change - psession->send_data_speed * psession->repeat / (psession->repeat + psession->send_count) / 2;
		}

	}
	else if ((VTCP_TIMER <= psession->rtt) && (VTCP_CALC_SN_INTERVAL(psession->current1, psession->sn) < 0)) //无额外的发送包
	{
		switch (psession->send_data_speed_level)
		{
		case 0: psession->send_data_speed_change = (psession->send_data_speed >> 0x2) + 1;	break;
		case 1: psession->send_data_speed_change = (psession->send_data_speed >> 0x3) + 1;	break;
		case 2: psession->send_data_speed_change = (psession->send_data_speed >> 0x4) + 1;	break;
		case 3: psession->send_data_speed_change = (psession->send_data_speed >> 0x5) + 1;	break;
		case 4: psession->send_data_speed_change = (psession->send_data_speed >> 0x6) + 1;	break;
		case 5: psession->send_data_speed_change = (psession->send_data_speed >> 0x7) + 1;	break;
		case 6: psession->send_data_speed_change = (psession->send_data_speed >> 0x8) + 1;	break;

		default:
			break;
		}

	}
	else //维持不变最好
	{
		psession->send_data_speed_change = 0;
	}

	if (psession->send_data_speed_change > 0 && psession->send_data_speed_change_prev < 0 ||
		psession->send_data_speed_change < 0 && psession->send_data_speed_change_prev > 0)
	{
		if (psession->send_data_speed_level < 6)
		{
			psession->send_data_speed_level++;
		}
	}

	psession->send_data_speed_prev = psession->send_data_speed;
	psession->send_data_speed = psession->send_data_speed_prev + psession->send_data_speed_change;

	if (psession->send_data_speed < (VTCP_PACKET_GROUP_COUNT << 16) / 1000) //最小带宽(3 KB/S)
	{
		psession->send_data_speed = (VTCP_PACKET_GROUP_COUNT << 16) / 1000;
	}

	if (psession->send_data_speed >(VTCP_PACKET_CACHE_COUNT << 16)) //最大带宽(256 * 1000 KB/S = 250 MB/S)
	{
		psession->send_data_speed = (VTCP_PACKET_CACHE_COUNT << 16);
	}

	psession->cwnd_prev_prev = psession->cwnd_prev;
	psession->cwnd_prev = psession->cwnd;

	psession->cwnd = (int)VTCP_CALC_S2W(psession->send_data_speed, psession->rtt);

	if (psession->cwnd < VTCP_PACKET_GROUP_COUNT)
	{
		psession->cwnd = VTCP_PACKET_GROUP_COUNT;
	}

	psession->update = psession->current1 + (7 - psession->send_data_speed_level) * (int)psession->cwnd;

	//////////////////////////////////////////////////////////////////////////

	//PCHAR flag = "**";

	//if (m_tcp_send_data_speed_change > 0) flag = "++";
	//if (m_tcp_send_data_speed_change < 0) flag = "--";

	//DBG_OUT(wsprintfA(msg, "带宽:[%3d] %s [%3d], 窗口=%3d, 拥塞:%3d, 间隔:%3d, 重发:%3d, 结余:%3d, 周期:%3d, 序号:%5d\r\n",
	//	int(m_tcp_send_data_speed_prev * 1000 / 65536), flag,
	//	int(m_tcp_send_data_speed * 1000 / 65536),
	//	int(m_tcp_send_data_cwnd_prev), int(sending), int(sendvar),
	//	int(m_tcp_send_data_count_repeat),
	//	int(m_tcp_send_data_speed_surplus >> 16),
	//	int(m_tcp_rtt),
	//	int(m_tcp_sn_send_min)));

	//////////////////////////////////////////////////////////////////////////

	psession->send_count = 0;
	psession->repeat = 0;

	// 	m_tcp_send_data_speed = VTCP_CALC_B2S(60);
	// 	m_tcp_send_data_speed = VTCP_CALC_B2S(60);
	// 	m_tcp_send_data_speed = VTCP_CALC_B2S(60);

	return(0);

}

inline void vtcp_session_close(struct vtcp_session *psession)
{
	psession->state = VTCP_STATE_CLOSED;
}

int vtcp_ontimer(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, unsigned int tickcount, unsigned int delta, unsigned int count)
{
	int d;

	if (psession->state == VTCP_STATE_CONNECTING)
	{
		if (int(tickcount - psession->connect_timeout_tick) > psession->connect_timeout)
		{
			//由于连接方在一段时间后没有正确答复或连接的主机没有反应，连接尝试失败
			pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_CONNECT, psession->address, sizeof(psession->address), NULL, NULL, VTCP_TIMEOUT);

			psession->state = VTCP_STATE_CLOSED;
		}
		else if (count % VTCP_TIMER_TIMES == 0)	//连接包频率为1秒1次
		{
			vtcp_send_connect(pvtcp, sid, psession->address, sizeof(psession->address));
			psession->last_send = tickcount;
		}
		else
		{
		}
	}
	else if (psession->state == VTCP_STATE_CONNECTED)
	{
		psession->send_data_speed_surplus += VTCP_TIMER * psession->send_data_speed;

		pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 1);
		vtcp_send_buffers(pvtcp, psession, sid, VTCP_PACKET_GROUP_COUNT, psession->rtt < VTCP_TIMER, tickcount);

		if (psession->send_data_speed_surplus > (VTCP_PACKET_GROUP_COUNT << 16))
		{
			psession->send_data_speed_surplus = VTCP_PACKET_GROUP_COUNT << 16;
		}
		if (VTCP_CALC_SN_INTERVAL(psession->update, psession->minimum1) < 0)
		{
			vtcp_update_speed(psession);
		}
		pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 0);

		if (int(tickcount - psession->last_send) > psession->keepalive_internal)
		{
			vtcp_send_sync(pvtcp, psession, sid, tickcount);
		}
	}
	else if (psession->state == VTCP_STATE_CONNRESET && (count % (VTCP_TIMER_TIMES / 10)) == 0)
	{
		vtcp_send_reset(pvtcp, psession, tickcount, sid);
	}

	if (psession->state == VTCP_STATE_CONNECTED || psession->state == VTCP_STATE_CONNRESET)
	{
		d = int(tickcount - psession->last_recv);
		if (d > 0)
		{
			if (d > psession->keepalive)
			{
				pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_TIMEOUT, psession->address, sizeof(psession->address), NULL, NULL, 0);

				psession->state = VTCP_STATE_CLOSED;

				pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 1);
				vtcp_queue_cancel(pvtcp, psession, sid, VTCP_PKTCMD_RESET);
				pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 0);
			}
		}
	}

	if (psession->state == VTCP_STATE_CLOSED)
	{
		pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 1);
		vtcp_session_close(psession);
		vtcp_session_uninitialize(psession);
		pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 0);
	}

	return(0);
}

void vtcp_onrecv_data(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size)
{
	struct vtcp_pkt_ext *ppe1;

	// 	if (m_tcp_recv_data_series_count_temp == 0)
	// 	{
	// 		m_tcp_recv_data_series_count_temp = 1; 
	// 		m_tcp_recv_data_series_time0 = pe->packet.body.data.urtt; 
	// 		m_tcp_recv_data_series_time1 = VTCP_GET_TIME();
	// 	}
	// 	else if (m_tcp_recv_data_series_time0 == pe->packet.body.data.urtt)
	// 	{
	// 		m_tcp_recv_data_series_count_temp++; 
	// 		m_tcp_recv_data_series_time0 = pe->packet.body.data.urtt; 
	// 		m_tcp_recv_data_series_time2 = VTCP_GET_TIME();
	// 	}
	// 	else 
	// 	{
	// 		if ((m_tcp_recv_data_series_count_temp > 1) && (m_tcp_recv_data_series_space >= VTCP_TIMER))
	// 		{
	// 			m_tcp_recv_data_series_count = m_tcp_recv_data_series_count_temp;
	// 			m_tcp_recv_data_series_space = m_tcp_recv_data_series_time2 - m_tcp_recv_data_series_time1;
	// 
	// 			if (m_tcp_recv_data_series_space > 0)
	// 			{
	// 				m_tcp_recv_data_series_speed = 1000 *  (m_tcp_recv_data_series_count -1) / m_tcp_recv_data_series_space;
	// 			}
	// 
	// 			//
	// 			//DBG_OUT(wsprintfA(msg,"band = %d KB/s\r\n",m_tcp_recv_data_series_speed));
	// 			//
	// 		}
	// 
	// 		m_tcp_recv_data_series_count_temp = 1; 
	// 		m_tcp_recv_data_series_time0 = pe->packet.body.data.urtt; 
	// 		m_tcp_recv_data_series_time1 = VTCP_GET_TIME();
	// 
	// 	}

	//_tprintf(_T("sn %d, minimum0 %d, maximum0 %d\r\n"), pp->data.sn, psession->minimum0, psession->maximum0);

	// 其实也可能发生的
	// 超前(不可能发生)
	// 滞后(有可能发生)
	if (VTCP_CALC_SN_INTERVAL(pp->data.sn, psession->maximum0) <= 0 && VTCP_CALC_SN_INTERVAL(pp->data.sn, psession->minimum0) > 0)
	{
		if (ppe1 = vtcp_packet_alloc(&psession->packet0, pp->data.sn))
		{
			ppe1->cb = pkt_size;
			ppe1->cbdata = pkt_size - (sizeof(struct vtcp_pkt) - VTCP_PACKET_DATA_SIZE);
			ppe1->cboffset = 0;
			ppe1->pkt.data.sn = pp->data.sn;

			memcpy(&ppe1->pkt, pp, pkt_size);
		}
		else
		{
			//DBG_OUT(wsprintfA(msg, "#REPEAT(sn=%d)\r\n", ppe->pkt.data.sn));
		}

		if (psession->current0 + 1 == pp->data.sn)//先发调用收通知
		{
			psession->current0++;

			while (vtcp_packet_get(&psession->packet0, psession->current0 + 1))
			{
				psession->current0++;
			}
		}

		if (VTCP_CALC_SN_INTERVAL(psession->current0, psession->minimum0) > 0)//先发调用收通知
		{
			vtcp_load_recv_buffers(pvtcp, psession, sid);
		}

		// 这个应该是毫无意义的
		psession->maximum0 = (psession->minimum0 + VTCP_PACKET_CACHE_COUNT);

		psession->recv_ack_freq = pp->data.ack_frequence;
	}

	if (++psession->recv_count >= psession->recv_ack_freq) //应答
	{
		psession->recv_count = 0;

		vtcp_send_dataack(pvtcp, psession, sid, pp->data.sn, pp->data.tickcount);

		// 		DWORD TT2 = VTCP_GET_TIME();
		// 		DWORD TT3 = TT2 -TT1; TT1 = TT2;
		// 
		// 		DBG_OUT(wsprintfA(msg,"SEND_ACK(this:%p,space:%d,ackfreq:%d)\r\n",this,TT3,m_tcp_recv_data_ack_freq));

	}

	psession->count_on_recv_data++;
}
// sender
void vtcp_onrecv_ack(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size, unsigned int tickcount)
{
	void *packet;
	unsigned int buffersize;

	pp->ack.tickcount = vtcp_read4bytes((const unsigned char *)&pp->ack.tickcount);
	pp->ack.sn = vtcp_read4bytes((const unsigned char *)&pp->ack.sn);
	pp->ack.minimum = vtcp_read2bytes((const unsigned char *)&pp->ack.minimum);
	pp->ack.maximum = vtcp_read2bytes((const unsigned char *)&pp->ack.maximum);
	pp->ack.current = vtcp_read2bytes((const unsigned char *)&pp->ack.current);

	psession->rtt = tickcount - pp->ack.tickcount;

	if (psession->send_data_speed < (VTCP_PACKET_GROUP_COUNT << 16) / (psession->rtt + 1))
	{
		psession->send_data_speed = (VTCP_PACKET_GROUP_COUNT << 16) / (psession->rtt + 1);
	}

	psession->cwnd = VTCP_CALC_S2W(psession->send_data_speed, psession->rtt);

	if (psession->cwnd < VTCP_PACKET_GROUP_COUNT)
	{
		psession->cwnd = VTCP_PACKET_GROUP_COUNT;
	}

	if (VTCP_CALC_SN_INTERVAL((int16_t)pp->ack.current + pp->ack.sn, psession->minimum1) > 0)
	{
		psession->minimum1 = (int16_t)pp->ack.current + pp->ack.sn;
		psession->maximum1 = (int16_t)pp->ack.maximum + pp->ack.sn;
	}

	buffersize = vtcp_packet_free(&psession->packet1, (int16_t)pp->ack.current + pp->ack.sn, VTCP_PACKET_CACHE_COUNT);

	// 根据 bits 释放
	buffersize += vtcp_packet_free(&psession->packet1, pp->ack.sn, pp->ack.bits, pp->ack.bitssize);

	packet = NULL;
	buffersize = pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_SENT, psession->address, sizeof(psession->address), &packet, NULL, buffersize);
	if (packet && buffersize)
	{
		struct vtcp_buffer *pb;

		pb = vtcp_queue_alloc(&psession->queue1);
		if (pb)
		{
			pb->buffer = (unsigned char *)packet;
			pb->length = buffersize;
			pb->offset = 0;
		}
	}

	vtcp_load_send_buffers(pvtcp, psession, sid);

	if (psession->minimum1 == psession->current1)
	{
		vtcp_send_buffers(pvtcp, psession, sid, 1, 0, tickcount);
	}

	psession->count_on_recv_data_ack++;
}
void vtcp_onrecv_sync(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size, unsigned int tickcount)
{
	vtcp_send_syncack(pvtcp, psession, sid, tickcount);

	psession->count_on_recv_sync++;
}
void vtcp_onrecv_syncack(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size, unsigned int tickcount)
{
	if (VTCP_CALC_SN_INTERVAL(pp->synack.current + pp->synack.sn, psession->minimum1) >= 0)//可能只是更新(wndmax)
	{
		psession->minimum1 = pp->synack.current + pp->synack.sn;
		psession->maximum1 = pp->synack.maximum + pp->synack.sn;
	}

	vtcp_packet_free(&psession->packet1, pp->synack.current + pp->synack.sn, VTCP_PACKET_CACHE_COUNT);

	if (psession->minimum1 == psession->current1)
	{
		vtcp_send_buffers(pvtcp, psession, sid, 1, 0, tickcount);
	}
	else
	{
	}

	psession->count_on_recv_sync_ack++;
}
void vtcp_onrecv_reset(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size, unsigned int tickcount)
{
	psession->state = VTCP_STATE_CLOSED;

	// 这里怕丢包, 原本执行了 3 次
	vtcp_send_resetack(pvtcp, psession, sid, tickcount);

	vtcp_queue_cancel(pvtcp, psession, sid, VTCP_PKTCMD_RESET);

	psession->count_on_recv_reset++;
}
void vtcp_onrecv_resetack(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size)
{
	psession->state = VTCP_STATE_CLOSED;

	vtcp_queue_cancel(pvtcp, psession, sid, VTCP_PKTCMD_RESET_ACK);

	psession->count_on_recv_reset_ack++;
}

int vtcp_onrecv(struct vtcp *pvtcp, struct vtcp_session *psession, unsigned int sid, struct vtcp_pkt *pp, unsigned int pkt_size, int errorcode, unsigned int tickcount)
{
	pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 1);

	psession->last_recv = tickcount;

	//_tprintf(_T("%p, state %d, index %d, cmd %x(%x), pkt_size %d\r\n"), psession, psession->state, pp->hdr.index, pp->hdr.cmd, LOBYTE(pp->hdr.cmd), pkt_size);

	if (psession->state == VTCP_STATE_CONNECTING)
	{
		if (VTCP_PKTCMD_CONNECT_ACK == (pp->hdr.cmd & 0xff))
		{
			psession->state = VTCP_STATE_CONNECTED;

			psession->index1 = vtcp_read2bytes((const unsigned char *)&pp->hdr + sizeof(struct vtcp_pkthdr));

			vtcp_packet_set_index(&psession->packet1, psession->index1, psession->sn, VTCP_PACKET_CACHE_COUNT);

			pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_CONNECT, psession->address, sizeof(psession->address), NULL, NULL, 0);

			// 继续关注信息
		}
		else if (VTCP_PKTCMD_CONNECT_ACK_REFUSE == (pp->hdr.cmd & 0xff))
		{
			psession->state = VTCP_STATE_BINDED;

			pvtcp->p_procedure(pvtcp->parameter, sid, pvtcp->fd, VTCP_CONNECT, psession->address, sizeof(psession->address), NULL, NULL, VTCP_PKTCMD_CONNECT_ACK_REFUSE);
		}
		else if (VTCP_PKTCMD_CONNECT_ACK_DELAY == (pp->hdr.cmd & 0xff))
		{
			// 继续关注信息
		}
	}
	else if (psession->state == VTCP_STATE_CONNECTED || psession->state == VTCP_STATE_CONNRESET)
	{
		if (errorcode)
		{
			vtcp_queue_cancel(pvtcp, psession, sid, errorcode);
		}
		else
		{
			switch (pp->hdr.cmd & 0xff)
			{
			case VTCP_PKTCMD_CONNECT:
			case VTCP_PKTCMD_CONNECT_ACK:
			case VTCP_PKTCMD_CONNECT_ACK_DELAY:
			case VTCP_PKTCMD_CONNECT_ACK_REFUSE:
				break;
			case VTCP_PKTCMD_DATA:
				vtcp_onrecv_data(pvtcp, psession, sid, pp, pkt_size);
				break;
			case VTCP_PKTCMD_DATA_ACK:
				vtcp_onrecv_ack(pvtcp, psession, sid, pp, pkt_size, tickcount);
				break;
			case VTCP_PKTCMD_SYNC:
				vtcp_onrecv_sync(pvtcp, psession, sid, pp, pkt_size, tickcount);
				break;
			case VTCP_PKTCMD_SYNC_ACK:
				vtcp_onrecv_syncack(pvtcp, psession, sid, pp, pkt_size, tickcount);
				break;
			case VTCP_PKTCMD_RESET:
				vtcp_onrecv_reset(pvtcp, psession, sid, pp, pkt_size, tickcount);
				break;
			case VTCP_PKTCMD_RESET_ACK:
				vtcp_onrecv_resetack(pvtcp, psession, sid, pp, pkt_size);
				break;

			default:
				break;
			}
		}
	}

	pvtcp->p_procedure(pvtcp->parameter, sid, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_SESSION, 0, NULL, NULL, 0);

	return(0);
}

// 通过套接字和地址获取实体
struct vtcp_session *vtcp_session_query(struct vtcp *pvtcp, unsigned int index, const unsigned char *address, unsigned int addresssize)
{
	struct vtcp_session *psession;

	psession = (struct vtcp_session *)vtcp_seek_session(pvtcp, index);
	if (psession)
	{
		if (memcmp(psession->address, address, addresssize) == 0)
		{
		}
		else
		{
			psession = NULL;
		}
	}
	return(psession);
}
struct vtcp_session *vtcp_session_find_unused(struct vtcp *pvtcp, unsigned int *index)
{
	struct vtcp_session *psession = pvtcp->sessions;
	struct vtcp_session *result = NULL;
	unsigned int i, j;

	for (i = 0; i < pvtcp->count; i++)
	{
		if (psession->state == VTCP_STATE_NULL || psession->state == VTCP_STATE_CLOSED)
		{
			result = psession;

			*index = i;

			break;
		}

		psession++;
	}
	return(result);
}
struct vtcp_session *vtcp_session_find(struct vtcp *pvtcp, unsigned int *index0, unsigned int index1, const unsigned char *address, unsigned int addresssize)
{
	struct vtcp_session *psession = pvtcp->sessions;
	struct vtcp_session *result = NULL;
	unsigned int i, j;
	unsigned int index = 0;

	for (i = 0; i < pvtcp->count; i++)
	{
		if (psession->state == VTCP_STATE_CONNECTED)
		{
			printf("index %d\r\n", index);

			// 远程的这个连接已经存在
			if (psession->index1 == index && memcmp(psession->address, address, addresssize) == 0)
			{
				result = psession;

				*index0 = index;

				break;
			}
		}
		else
		{
			if (psession->state == VTCP_STATE_NULL || psession->state == VTCP_STATE_CLOSED)
			{
				result = psession;

				*index0 = index;
			}
		}

		index++;

		psession++;
	}
	return(result);
}

unsigned int vtcp_connect(struct vtcp *pvtcp, const unsigned char *address, unsigned int addresssize, unsigned int tickcount)
{
	struct vtcp_session *psession;
	unsigned int index;

	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, NULL, 0, NULL, NULL, 1);
	psession = vtcp_session_find_unused(pvtcp, &index);
	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, NULL, 0, NULL, NULL, 0);
	if (psession)
	{
		psession->state = VTCP_STATE_CONNECTING;

		memset(psession->address, 0, sizeof(psession->address));
		memcpy(psession->address, address, addresssize);

		psession->connect_timeout_tick = tickcount;

		vtcp_send_connect(pvtcp, index, address, addresssize);
	}
	else
	{
		index = -1;
	}
	return(index);
}

int vtcp_send_accept(struct vtcp *pvtcp, const unsigned char *address, unsigned int addresssize, unsigned short cmd, unsigned short index0, unsigned short index1)
{
	struct vtcp_pkt_ext ppe[1];

	ppe->cb = sizeof(struct vtcp_pkthdr) + sizeof(uint16_t);
	ppe->cbdata = 0;
	ppe->cboffset = 0;

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.cmd, cmd);
	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, index1);

	vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr + sizeof(struct vtcp_pkthdr), index0);

	pvtcp->p_procedure(pvtcp->parameter, index0, pvtcp->fd, VTCP_SEND, address, addresssize, NULL, (unsigned char *)&ppe->pkt, ppe->cb);

	return(0);
}

void vtcp_door_onrecv(struct vtcp *pvtcp, struct vtcp_pkt *pp, unsigned int pkt_size, const unsigned char *address, unsigned int addresssize, int errorcode, unsigned int tickcount)
{
	struct vtcp_session *psession;
	struct vtcp_door *pdoor;
	unsigned int index = 0;
	unsigned int cmd = 0;

	if (errorcode)
	{
		;
	}
	else
	{
		pdoor = vtcp_search_door(pvtcp, address, addresssize);

		psession = vtcp_session_find(pvtcp, &index, pp->hdr.index, address, addresssize);
		if (psession == NULL)
		{
			// 没有找到

			cmd = VTCP_PKTCMD_RESET;
		}
		else
		{
			//已经连接

			if (psession->state != VTCP_STATE_CONNECTED)
			{
				psession->state = VTCP_STATE_CONNECTED;

				psession->index1 = pp->hdr.index;

				psession->last_send = tickcount;
				psession->last_recv = tickcount;

				memcpy(psession->address, address, addresssize);

				pvtcp->p_procedure(pvtcp->parameter, index, pvtcp->fd, VTCP_ACCEPT, address, addresssize, NULL, NULL, 0);

				psession->last_send = tickcount;
			}

			cmd = VTCP_PKTCMD_CONNECT_ACK;
		}

		if (cmd)
		{
			vtcp_send_accept(pvtcp, address, addresssize, cmd, index, pp->hdr.index);
		}
	}
}

int vtcp_onrecv(struct vtcp *pvtcp, struct vtcp_pkt *pp, unsigned int pkt_size, const unsigned char *address, unsigned int addresssize, int errorcode, unsigned int tickcount)
{
	struct vtcp_session *psession;
	unsigned int sid;
	int result = 0;

	sid = pp->hdr.index;

	if (psession = vtcp_session_query(pvtcp, sid, address, addresssize))
	{
		vtcp_onrecv(pvtcp, psession, sid, pp, pkt_size, 0, tickcount);

		result = 1;
	}

	return(result);
}

int vtcp_session_timer(struct vtcp *pvtcp, unsigned int tickcount, unsigned int delta, unsigned int count)
{
	struct vtcp_session *psession = pvtcp->sessions;
	unsigned int sid = 0;
	unsigned int i, j;

	for (i = 0; i < pvtcp->count; i++)
	{
		vtcp_ontimer(pvtcp, psession, sid, tickcount, delta, count);
		sid++;

		psession++;
	}

	return(0);
}