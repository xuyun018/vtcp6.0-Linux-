#include "vtcp_packet.h"

unsigned int vtcp_read2bytes(const unsigned char *buffer)
{
	unsigned int result;

	result = buffer[0];
	result |= (buffer[1] << 8);
	return(result);
}
void vtcp_write2bytes(unsigned char *buffer, unsigned int value)
{
	buffer[0] = value;
	buffer[1] = value >> 8;
}
unsigned int vtcp_read4bytes(const unsigned char *buffer)
{
	unsigned int result;

	result = buffer[0];
	result |= (buffer[1] << 8);
	result |= (buffer[2] << 16);
	result |= (buffer[3] << 24);
	return(result);
}
void vtcp_write4bytes(unsigned char *buffer, unsigned int value)
{
	buffer[0] = value;
	buffer[1] = value >> 8;
	buffer[2] = value >> 16;
	buffer[3] = value >> 24;
}

void vtcp_packet_initialize(struct vtcp_packet *pp)
{
	unsigned int i;

	pp->count = 0;
	for (i = 0; i < VTCP_PACKET_CACHE_COUNT; i++)
	{
		// 标记为空包
		pp->packets[i].cb = 0;
	}
}
void vtcp_packet_uninitialize(struct vtcp_packet *pp)
{
	pp->count = 0;
	unsigned int i;

	pp->count = 0;
	for (i = 0; i < VTCP_PACKET_CACHE_COUNT; i++)
	{
		// 标记为空包
		pp->packets[i].cb = 0;
	}
}

struct vtcp_pkt_ext *vtcp_packet_alloc(struct vtcp_packet *pp, unsigned int sn)
{
	struct vtcp_pkt_ext *ppe;

	ppe = &pp->packets[sn & VTCP_PACKET_CACHE_COUNT_MASK];
	if (ppe->cb == 0)
	{
		//初试化为最大包大小
		ppe->cb = sizeof(struct vtcp_pkt);
		ppe->cbdata = 0;
		ppe->cboffset = 0;
		ppe->pkt.hdr.cmd = 0;
		ppe->pkt.hdr.index = 0;

		pp->count++;
	}
	else
	{
		ppe = NULL;
	}

	return(ppe);
}
struct vtcp_pkt_ext *vtcp_packet_get(struct vtcp_packet *pp, unsigned int sn)
{
	struct vtcp_pkt_ext *ppe;

	ppe = &pp->packets[sn & VTCP_PACKET_CACHE_COUNT_MASK];
	if (ppe->pkt.data.sn == sn && ppe->cb)
	{
	}
	else
	{
		ppe = NULL;
	}
	return(ppe);
}
// 连接完成之后需要更新之前数据包的信息
unsigned int vtcp_packet_set_index(struct vtcp_packet *pp, unsigned int index, unsigned int sn, unsigned int count)
{
	struct vtcp_pkt_ext *ppe;
	unsigned int i;
	unsigned int result = 0;

	for (i = 0; i < count; i++)
	{
		ppe = &pp->packets[(sn - i) & VTCP_PACKET_CACHE_COUNT_MASK];
		if (ppe->cb)
		{
			if (ppe->pkt.data.sn == sn - i)
			{
				vtcp_write2bytes((unsigned char *)&ppe->pkt.hdr.index, index);

				result++;
			}
			else
			{
				break;
			}
		}
	}

	return(result);
}
// 只是设置个标志
unsigned int vtcp_packet_free(struct vtcp_packet *pp, unsigned int sn)
{
	struct vtcp_pkt_ext *ppe;
	unsigned int result = 0;

	ppe = &pp->packets[sn & VTCP_PACKET_CACHE_COUNT_MASK];
	if (ppe->pkt.data.sn == sn && ppe->cb)
	{
		result = ppe->cb;

		ppe->cb = 0;
		pp->count--;
	}
	return(result);
}
// 释放多个
unsigned int vtcp_packet_free(struct vtcp_packet *pp, unsigned int sn, unsigned int count)
{
	struct vtcp_pkt_ext *ppe;
	unsigned int i;
	unsigned int result = 0;

	for (i = 0; i < count; i++)
	{
		ppe = &pp->packets[(sn - i) & VTCP_PACKET_CACHE_COUNT_MASK];
		if (ppe->cb)
		{
			if (ppe->pkt.data.sn == sn - i)
			{
				result += ppe->cb;

				ppe->cb = 0;
				pp->count--;
			}
			else
			{
				break;
			}
		}
	}

	return(result);
}
unsigned int vtcp_packet_free(struct vtcp_packet *pp, unsigned int sn, uint8_t *bits, unsigned char bitssize)
{
	unsigned int i;
	unsigned int j;
	unsigned int result = 0;

	// 有多余的操作
	for (j = 0; j < bitssize; j++)
	{
		for (i = 0; i < 8; i++)
		{
			if (bits[j] & (1 << i))
			{
				result += vtcp_packet_free(pp, sn);
			}

			sn--;
		}
	}

	return(result);
}
unsigned int vtcp_packet_makebits(struct vtcp_packet *pp, unsigned int sn, unsigned int minimum, uint8_t *bits)
{
	struct vtcp_pkt_ext *ppe;
	unsigned int i;
	unsigned int j;
	unsigned int index;

	for (j = 0;; j++)
	{
		bits[j] = 0;

		for (i = 0; i < 8; i++)
		{
			index = sn & VTCP_PACKET_CACHE_COUNT_MASK;

			ppe = &pp->packets[index];
			if (ppe->pkt.data.sn == sn)
			{
				bits[j] |= (1 << i);
			}
			else if (ppe->cb == 0)
			{
				// do nothing
			}
			else
			{
			}

			if (sn == minimum)
			{
				if (i)
				{
					j++;
				}

				return(j);
			}

			sn--;
		}
	}

	return(j);
}