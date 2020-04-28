#ifndef VTCP_SESSION_H
#define VTCP_SESSION_H

#include "vtcp.h"
#include "vtcp_queue.h"
#include "vtcp_packet.h"

#include <string.h>
#include <stdio.h>

#define VTCP_SESSION_FLAG_WORKING									0x01

unsigned int vtcp_connect(struct vtcp *pvtcp, const unsigned char *address, unsigned int addresssize, unsigned int tickcount);
int vtcp_send(struct vtcp *pvtcp, unsigned int sid, const unsigned char *buffer, unsigned int length, unsigned int tickcount);

int vtcp_session_timer(struct vtcp *pvtcp, unsigned int tickcount, unsigned int delta, unsigned int count);

void vtcp_door_onrecv(struct vtcp *pvtcp, struct vtcp_pkt *pp, unsigned int pkt_size, const unsigned char *address, unsigned int addresssize, int errorcode, unsigned int tickcount);
int vtcp_onrecv(struct vtcp *pvtcp, struct vtcp_pkt *pp, unsigned int pkt_size, const unsigned char *address, unsigned int addresssize, int errorcode, unsigned int tickcount);


#endif