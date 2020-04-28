#include "xyudp.h"
//---------------------------------------------------------------------------
int xy_setnonblocking(int fd, int flags, int nonblock)
{
	if (flags == -1)
	{
		flags = fcntl(fd, F_GETFL, 0);
	}
	if (flags != -1)
	{
		if (nonblock)
		{
			if ((flags & O_NONBLOCK) == 0)
			{
				flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
			}
		}
		else
		{
			if ((flags & O_NONBLOCK) != 0)
			{
				flags = fcntl(fd, F_SETFL, flags ^ O_NONBLOCK);
			}
		}
	}
	return(flags);
}

unsigned int xyudp_proc(struct xyudp* pudp)
{
	xyudp_procedure procedure = pudp->procedure;
	fd_set fds0;
	fd_set fds2;
	struct timeval tv;
	int maximum;
	char buffer[2048];
	int length;
	struct sockaddr_in6 sai6;
	socklen_t addresssize;
	int addresslength;
	int fd = pudp->fd;
	const void *parameter = pudp->parameter;
	const void *context = pudp->context;
	int errorcode = 0;
	int value;
	int flag;

	xy_setnonblocking(fd, -1, 1);

	while (errorcode == 0)
	{
		maximum = 0;

		FD_ZERO(&fds0);
		FD_ZERO(&fds2);

		maximum = fd > maximum ? fd : maximum;

		FD_SET(fd, &fds0);
		FD_SET(fd, &fds2);

		tv.tv_sec = 0;
		tv.tv_usec = pudp->milliseconds;

		value = select(maximum + 1, &fds0, NULL, &fds2, &tv);
		switch (value)
		{
		case 0:
			errorcode = procedure(pudp, parameter, context, fd, XYSOCKET_TIMEOUT, NULL, NULL, NULL, 0);
			if (errorcode)
			{
				break;
			}
		case -1:
		default:
			if (value == 0 || value != -1 && FD_ISSET(fd, &fds0))
			{
				do
				{
					flag = 0;

					value = sizeof(buffer);

					//addresssize = sizeof(sai6);
					addresssize = sizeof(struct sockaddr_in);
					length = recvfrom(fd, buffer, value, 0, (struct sockaddr*) & sai6, &addresssize);
					if (length > 0)
					{
						addresslength = addresssize;
						errorcode = procedure(pudp, parameter, context, fd, XYSOCKET_RECV, (struct sockaddr*) & sai6, &addresslength, buffer, &length);
						if (errorcode == 0)
						{
							flag = 1;
						}
					}
					else
					{
						if (length == 0)
						{
							errorcode = XYSOCKET_ERROR_FAILED;
						}
						else
						{
							// length == SOCKET_ERROR
							errorcode = errno;
							if (errorcode == EWOULDBLOCK || errorcode == ETIMEDOUT)
							{
								errorcode = 0;
							}
							else
							{
								printf("bad %d\r\n", errorcode);
							}
						}
					}
				} while (flag);
			}
			else
			{
				if (FD_ISSET(fd, &fds2))
				{
					errorcode = XYSOCKET_ERROR_FAILED;
				}
			}
			break;
		}
	}

	procedure(pudp, parameter, context, fd, XYSOCKET_CLOSE, NULL, NULL, NULL, &errorcode);

	return(0);
}
//---------------------------------------------------------------------------
