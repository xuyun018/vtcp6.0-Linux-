
#include "vtcp.h"
#include "vtcp_queue.h"
#include "vtcp_packet.h"

void vtcp_session_initialize(struct vtcp_session *psession)
{
	psession->state = VTCP_STATE_NULL;
	psession->flags = 0;

	// 逗留
	psession->linger = 1;
	//逗留时间
	psession->linger_timeout = 15000;
	psession->linger_timeout_tick = 0;

	psession->keepalive = 30000;
	psession->keepalive_internal = 5000;

	psession->connect_timeout = 30000;

	psession->count_do_send_data = 0;
	psession->count_do_send_data_ack = 0;
	psession->count_do_send_data_ack_lost = 0;
	psession->count_do_send_data_repeat = 0;
	psession->count_do_send_sync = 0;
	psession->count_do_send_sync_ack = 0;
	psession->count_do_send_reset = 0;
	psession->count_do_send_reset_ack = 0;

	psession->count_on_recv_data = 0;
	psession->count_on_recv_data_ack = 0;
	psession->count_on_recv_data_ack_lost = 0;
	psession->count_on_recv_sync = 0;
	psession->count_on_recv_sync_ack = 0;
	psession->count_on_recv_reset = 0;
	psession->count_on_recv_reset_ack = 0;

	psession->count_send_bytes = 0;

	psession->sn = 0;

	psession->current1 = 0;
	psession->maximum1 = VTCP_PACKET_CACHE_COUNT;
	psession->minimum1 = 0;
	psession->update = 1;

	psession->current0 = 0;
	psession->maximum0 = VTCP_PACKET_CACHE_COUNT;
	psession->minimum0 = 0;

	psession->recv_count = 0;

	psession->rtt = 1000;
	psession->rtt_prev = -1;

	psession->send_data_speed = VTCP_PACKET_GROUP_COUNT * 65536 / 1000;
	psession->send_data_speed_prev = psession->send_data_speed;
	psession->send_data_speed_surplus = VTCP_PACKET_GROUP_COUNT * 65536;
	psession->send_data_speed_change = 0;
	psession->send_data_speed_change_prev = 0;
	psession->send_data_speed_level = 1;
	psession->cwnd_prev_prev = VTCP_PACKET_GROUP_COUNT;
	psession->cwnd_prev = VTCP_PACKET_GROUP_COUNT;
	psession->cwnd = VTCP_PACKET_GROUP_COUNT;
	psession->repeat = 0;
	psession->send_count = 0;

	psession->recv_count = 0;
	psession->recv_ack_freq = 0;

	vtcp_queue_initialize(&psession->queue1);

	vtcp_packet_initialize(&psession->packet0);
	vtcp_packet_initialize(&psession->packet1);
}
void vtcp_session_uninitialize(struct vtcp_session *psession)
{
	vtcp_queue_uninitialize(&psession->queue1);

	vtcp_packet_uninitialize(&psession->packet0);
	vtcp_packet_uninitialize(&psession->packet1);
}

void vtcp_sessions_clear(struct vtcp *pvtcp)
{
	struct vtcp_session *psession = pvtcp->sessions;
	unsigned int i;

	for (i = 0; i < pvtcp->count; i++)
	{
		vtcp_session_uninitialize(psession);

		psession++;
	}

	pvtcp->p_procedure(pvtcp->parameter, NULL, 0, VTCP_RELEASE, NULL, 0, (void **)&pvtcp->sessions, NULL, 0);

	pvtcp->sessions = NULL;
	pvtcp->count = 0;
}

unsigned int vtcp_set_ranks(struct vtcp *pvtcp, unsigned int count)
{
	struct vtcp_session *psession;
	unsigned int result = 0;
	unsigned int i;

	if (count && pvtcp->count != count)
	{
		vtcp_sessions_clear(pvtcp);

		pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_REQUEST, NULL, 0, (void **)&pvtcp->sessions, NULL, sizeof(struct vtcp_session) * count);
		if (pvtcp->sessions)
		{
			psession = pvtcp->sessions;
			for (i = 0; i < count; i++)
			{
				vtcp_session_initialize(psession);

				result++;

				psession++;
			}

			pvtcp->count = count;
		}
	}

	return(result);
}

void vtcp_initialize(struct vtcp *pvtcp, void *parameter, t_vtcp_procedure p_procedure)
{
	pvtcp->door_count = 0;

	pvtcp->sessions = NULL;
	pvtcp->count = 0;

	pvtcp->parameter = parameter;

	pvtcp->p_procedure = p_procedure;

	pvtcp->fd = (unsigned int)-1;
}
void vtcp_uninitialize(struct vtcp *pvtcp)
{
	vtcp_sessions_clear(pvtcp);
}

struct vtcp_door *vtcp_search_door(struct vtcp *pvtcp, const unsigned char *address, unsigned int addresssize)
{
	struct vtcp_door *pdoor = NULL;
	unsigned int i;

	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_DOOR, 0, NULL, NULL, 1);
	for (i = 0; i < pvtcp->door_count; i++)
	{
		pdoor = &pvtcp->doors[i];
		if (pvtcp->p_procedure(pvtcp->parameter, -1, pvtcp->fd, VTCP_ADDRESSES_COMPARE, pdoor->address, sizeof(pdoor->address), NULL, (unsigned char *)address, addresssize) == 0)
		{
			// 找到
			break;
		}
		pdoor = NULL;
	}
	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_DOOR, 0, NULL, NULL, 0);

	return(pdoor);
}

struct vtcp_door *vtcp_door_open(struct vtcp *pvtcp, void *pointer, const unsigned char *address, unsigned int addresssize)
{
	struct vtcp_door *pdoor = NULL;
	unsigned int i;

	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_DOOR, 0, NULL, NULL, 1);

	for (i = 0; i < pvtcp->door_count; i++)
	{
		pdoor = &pvtcp->doors[i];
		if (pvtcp->p_procedure(pvtcp->parameter, NULL, pvtcp->fd, VTCP_ADDRESSES_COMPARE, pdoor->address, sizeof(pdoor->address), &pointer, (unsigned char *)address, addresssize) == 0)
		{
			// 找到
			break;
		}
		pdoor = NULL;
	}

	if (pdoor == NULL)
	{
		if (pvtcp->door_count < sizeof(pvtcp->doors) / sizeof(pvtcp->doors[0]))
		{
			pdoor = &pvtcp->doors[pvtcp->door_count++];

			memset(pdoor->address, 0, sizeof(pdoor->address));
			memcpy(pdoor->address, address, addresssize);
		}
	}

	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_DOOR, 0, NULL, NULL, 0);

	return(pdoor);
}
int vtcp_door_close(struct vtcp *pvtcp, struct vtcp_door *pdoor)
{
	struct vtcp_door *pdoor0;
	struct vtcp_door *pdoor1;
	unsigned int i;
	int result = 0;

	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_DOOR, 0, NULL, NULL, 1);

	i = pdoor - &pvtcp->doors[0];
	if (i < pvtcp->door_count)
	{
		pvtcp->door_count--;

		if (i < pvtcp->door_count)
		{
			pdoor0 = &pvtcp->doors[i];
			pdoor1 = &pvtcp->doors[pvtcp->door_count];

			memcpy(pdoor0->address, pdoor1->address, sizeof(pdoor0->address));
		}

		result = 1;
	}

	pvtcp->p_procedure(pvtcp->parameter, -1, 0, VTCP_LOCK, (const unsigned char *)VTCP_LOCK_DOOR, 0, NULL, NULL, 0);

	return(result);
}