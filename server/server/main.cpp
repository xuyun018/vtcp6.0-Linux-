#include "../../xyudp.h"
#include "../../vtcp.h"

#include "../../vtcp_packet.h"
#include "../../vtcp_session.h"

#include <sys/time.h>
// bzero, memcopy ...
#include <string.h>
// signal
#include <signal.h>

// malloc
#include <stddef.h>
#include <stdlib.h>

struct vtcp_context
{
	unsigned int count;
	unsigned int total;
	unsigned int tickcount0;
	unsigned int tickcount1;
};

/* get system time */
void itimeofday(long* sec, long* usec)
{
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
}

/* get clock in millisecond 64 */
long long iclock64(void)
{
	long s, u;
	long long value;
	itimeofday(&s, &u);
	value = ((long long)s) * 1000 + (u / 1000);
	return value;
}

unsigned int iclock()
{
	return (unsigned int)(iclock64() & 0xfffffffful);
}

unsigned int ipv4_2_buffer(const struct sockaddr_in* psai, unsigned char* buffer)
{
	unsigned int result = 1;

	buffer[result++] = psai->sin_family;
	buffer[result++] = psai->sin_port;
	buffer[result++] = psai->sin_port >> 8;
	memcpy(buffer + result, &psai->sin_addr.s_addr, 4);
	result += 4;
	buffer[0] = result;

	return(result);
}
unsigned int buffer_2_ipv4(const unsigned char* buffer, struct sockaddr_in* psai)
{
	unsigned int result = 1;

	memset(psai, 0, sizeof(struct sockaddr_in));

	psai->sin_family = buffer[result++];
	psai->sin_port = buffer[result++];
	psai->sin_port |= buffer[result++] << 8;
	memcpy(&psai->sin_addr.s_addr, buffer + result, 4);
	result += 4;
	return(result);
}
unsigned int ipv6_2_buffer(const struct sockaddr_in6* psai, unsigned char* buffer)
{
	unsigned int i;
	unsigned int result = 1;

	// ¿ÉÄÜ²»¶Ô, »¹ÓÐ±ðµÄÃ»ÓÐÐ´

	buffer[result++] = psai->sin6_family;
	buffer[result++] = psai->sin6_port;
	buffer[result++] = psai->sin6_port >> 8;
	memcpy(buffer + result, &psai->sin6_addr, 16);
	result += 16;
	buffer[0] = result;
	return(result);
}
unsigned int buffer_2_ipv6(const unsigned char* buffer, struct sockaddr_in6* psai)
{
	unsigned int i;
	unsigned int result = 1;

	memset(psai, 0, sizeof(struct sockaddr_in6));

	psai->sin6_family = buffer[result++];
	psai->sin6_port = buffer[result++];
	psai->sin6_port |= buffer[result++] << 8;
	memcpy(&psai->sin6_addr, buffer + result, 16);
	result += 16;
	return(result);
}

// ÍêÈ«Æ¥ÅäÓÃ
int addresses_compare0(const void* parameter, const unsigned char* address0, const unsigned char* address1)
{
	unsigned int i;
	int compareresult;

	compareresult = address0[0] - address1[0];
	if (compareresult == 0)
	{
		compareresult = address0[1] - address1[1];
		if (compareresult == 0)
		{
			compareresult = address0[3] - address1[3];
			if (compareresult == 0)
			{
				compareresult = address0[2] - address1[2];
			}

			if (compareresult == 0)
			{
				switch (address0[1])
				{
				case AF_INET:
					for (i = 0; compareresult == 0 && i < 4; i++)
					{
						compareresult = address0[4 + i] - address1[4 + i];
					}
					break;
				case AF_INET6:
					for (i = 0; compareresult == 0 && i < 16; i++)
					{
						compareresult = address0[4 + i] - address1[4 + i];
					}
					break;
				default:
					break;
				}
			}
		}
	}
	return(compareresult);
}
// ÅÐ¶ÏÊÇ·ñÉÏÏßµÄµØÖ·ÓÃ
int addresses_compare1(const void* parameter, const unsigned char* address0, const unsigned char* address1)
{
	unsigned int i;
	int compareresult;

	compareresult = address0[0] - address1[0];
	if (compareresult == 0)
	{
		compareresult = address0[1] - address1[1];
		if (compareresult == 0)
		{
			if (address0[2] != 0 || address0[3] != 0)
			{
				compareresult = address0[3] - address1[3];
				if (compareresult == 0)
				{
					compareresult = address0[2] - address1[2];
				}
			}

			if (compareresult == 0)
			{
				switch (address0[1])
				{
				case AF_INET:
					for (i = 0; i < 4; i++)
					{
						if (address0[4 + i])
						{
							break;
						}
					}
					if (i < 4)
					{
						for (i = 0; compareresult == 0 && i < 4; i++)
						{
							compareresult = address0[4 + i] - address1[4 + i];
						}
					}
					break;
				case AF_INET6:
					for (i = 0; i < 16; i++)
					{
						if (address0[4 + i])
						{
							break;
						}
					}
					if (i < 4)
					{
						for (i = 0; compareresult == 0 && i < 16; i++)
						{
							compareresult = address0[4 + i] - address1[4 + i];
						}
					}
					break;
				default:
					break;
				}
			}
		}
	}
	return(compareresult);
}

int vtcp_procedure(void* parameter, unsigned int sid, unsigned int fd, unsigned char number, const unsigned char* address, unsigned int addresssize, void** packet, unsigned char* buffer, unsigned int bufferlength)
{
	struct vtcp_context* pcontext = (struct vtcp_context*)parameter;
	int result = 0;
	struct sockaddr_in sai;

	switch (number)
	{
	case VTCP_SEND:
		buffer_2_ipv4(address, &sai);
		sendto(fd, buffer, bufferlength, 0, (struct sockaddr*) & sai, sizeof(sai));
		break;
	case VTCP_LOAD_SEND:
		break;
	case VTCP_SENT:
		break;
	case VTCP_RECV:
		//_tprintf(_T("Receive %d\r\n"), bufferlength);
		pcontext->total += bufferlength;
		break;
	case VTCP_CONNECT:
		break;
	case VTCP_ACCEPT:
		printf("Accept %d\r\n", bufferlength);
		break;
	case VTCP_LISTEN:
		break;
	case VTCP_ADDRESSES_COMPARE:
		printf("packet %p\r\n", packet);
		if (packet)
		{
			result = addresses_compare0(*packet, address, buffer);

			printf("address compare %d\r\n", result);
		}
		else
		{
			result = addresses_compare1(NULL, address, buffer);
		}
		break;
	case VTCP_ADDRESS_READ:
		ipv4_2_buffer((const struct sockaddr_in *)address, buffer);
		break;
	case VTCP_REQUEST:
		*packet = malloc(bufferlength);
		break;
	case VTCP_RELEASE:
		free(*packet);
		*packet = NULL;
		break;
	case VTCP_LOCK:
		if (bufferlength)
		{
			//EnterCriticalSection(&pengine->critical_sections[(unsigned int)address]);
		}
		else
		{
			//LeaveCriticalSection(&pengine->critical_sections[(unsigned int)address]);
		}
		break;
	default:
		break;
	}

	return(result);
}

int p_procedure(struct xyudp* pudp, const void* parameter, const void* context, int fd, unsigned char number, const struct sockaddr* psa, int* salen, const char* buffer, int* len)
{
	struct vtcp* pvtcp = (struct vtcp*)parameter;
	struct vtcp_context* pcontext = (struct vtcp_context*)context;
	int l;
	int result = 0;
	unsigned int tickcount;

	switch (number)
	{
	case XYSOCKET_CLOSE:
		break;
	case XYSOCKET_RECV:
		//printf("recv len %d\r\n", *len);
		{
			struct vtcp_door door;
			struct vtcp_pkt* pp;

			tickcount = iclock();

			memset(door.address, 0, sizeof(door.address));
			pvtcp->p_procedure(pvtcp->parameter, NULL, pvtcp->fd, VTCP_ADDRESS_READ, (const unsigned char*)psa, *salen, NULL, door.address, sizeof(door.address));

			pp = (struct vtcp_pkt*)buffer;

			pp->hdr.cmd = vtcp_read2bytes((const unsigned char*)&pp->hdr.cmd);
			pp->hdr.index = vtcp_read2bytes((const unsigned char*)&pp->hdr.index);

			switch (pp->hdr.cmd & 0xff)
			{
			case VTCP_PKTCMD_CONNECT:
				vtcp_door_onrecv(pvtcp, pp, *len, door.address, sizeof(door.address), 0, tickcount);
				break;
			default:
				vtcp_onrecv(pvtcp, pp, *len, door.address, sizeof(door.address), 0, tickcount);
				break;
			}
		}
	case XYSOCKET_TIMEOUT:
		tickcount = iclock();

		if (tickcount - pcontext->tickcount0 > 10)
		{
			vtcp_session_timer(pvtcp, tickcount, tickcount - pcontext->tickcount0, pcontext->count++);
			pcontext->tickcount0 = tickcount;
		}

		if (tickcount - pcontext->tickcount1 > 1000)
		{
			unsigned long long speed;

			l = tickcount - pcontext->tickcount1;
			speed = pcontext->total;
			speed *= 1000;
			speed /= l;
			l = speed;

			printf("speed %d\r\n", l);

			pcontext->total = 0;
			pcontext->tickcount1 = tickcount;
		}
		break;
	default:
		break;
	}
	return(result);
}

int main(int argc, char* argv[])
{
	struct xyudp pudp[1];
	struct vtcp pvtcp[1];
	struct vtcp_door* pdoor;
	unsigned char address[20];
	unsigned int addresssize;
	struct sockaddr_in sai;
	struct vtcp_context context[1];
	int fd;

	signal(SIGPIPE, SIG_IGN);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd != -1)
	{
		bzero(&sai, sizeof(sai));

		sai.sin_family = AF_INET;
		sai.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY
		sai.sin_port = htons(2047);

		if (bind(fd, (struct sockaddr*) & sai, sizeof(sai)) == 0)
		{
			vtcp_initialize(pvtcp, context, vtcp_procedure);

			vtcp_set_ranks(pvtcp, 1);

			pvtcp->fd = fd;

			memset(&sai, 0, sizeof(sai));
			sai.sin_family = AF_INET;
			sai.sin_addr.s_addr = INADDR_ANY;
			//sai.sin_port = htons(1980);
			//if (bind(pvtcp->fd, (const sockaddr *)&sai, sizeof(sai)) != SOCKET_ERROR)
			{
				pudp->parameter = (void*)pvtcp;
				pudp->context = (void*)context;
				pudp->procedure = p_procedure;
				pudp->fd = fd;
				pudp->milliseconds = 1;

				context->count = 0;
				context->total = 0;
				context->tickcount0 = 0;
				context->tickcount1 = 0;

				sai.sin_port = 0;

				addresssize = ipv4_2_buffer((const struct sockaddr_in*) & sai, address);
				pdoor = vtcp_door_open(pvtcp, NULL, address, addresssize);
				if (pdoor)
				{
					xyudp_proc(pudp);

					vtcp_door_close(pvtcp, pdoor);
				}
			}

			vtcp_uninitialize(pvtcp);
		}

		close(fd);
	}

	return(0);
}