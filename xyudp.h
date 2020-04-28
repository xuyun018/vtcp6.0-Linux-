
#ifndef XYUDP_H
#define XYUDP_H
//---------------------------------------------------------------------------
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define XYSOCKET_CLOSE														0
#define XYSOCKET_RECV														2
#define XYSOCKET_TIMEOUT													4

#define XYSOCKET_ERROR_FAILED												5
//---------------------------------------------------------------------------
struct xyudp;
typedef int (*xyudp_procedure)(struct xyudp *, const void *, const void *, int, unsigned char, const struct sockaddr *, int *, const char *, int *);
//---------------------------------------------------------------------------
struct xyudp
{
	const void *parameter;
	const void *context;

	xyudp_procedure procedure;

	int fd;
	int milliseconds;
};
//---------------------------------------------------------------------------
unsigned int xyudp_proc(struct xyudp* pudp);
//---------------------------------------------------------------------------
#endif