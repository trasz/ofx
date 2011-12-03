#include <sys/queue.h>
#include <netinet/in.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "monitoring.h"
#include "openflow.h"
#include "ofproto.h"
#include "ofswitch.h"
#include "packet.h"
#include "topology.h"

static const char *oftypes[] = {
	"HELLO",
	"ERROR",
	"ECHO_REQUEST",
	"ECHO_REPLY",
	"VENDOR",
	"FEATURES_REQUEST",
	"FEATURES_REPLY",
	"GET_CONFIG_REQUEST",
	"GET_CONFIG_REPLY",
	"SET_CONFIG",
	"PACKET_IN",
	"FLOW_REMOVED",
	"PORT_STATUS",
	"PACKET_OUT",
	"FLOW_MOD",
	"PORT_MOD",
	"STATS_REQUEST",
	"STATS_REPLY",
	"BARRIER_REQUEST",
	"BARRIER_REPLY",
	"QUEUE_GET_CONFIG_REQUEST",
	"QUEUE_GET_CONFIG_REPLY"};

static const char *
oftype_str(int type)
{
	if (type < 0 || type >= sizeof(oftypes))
		errx(1, "invalid oftype %d", type);
	return (oftypes[type]);
}

static struct packet *
ofproto_read(int fd)
{
	struct ofp_header *ofph;
	struct packet *p;
	uint16_t len;

	p = p_alloc();
	ofph = (struct ofp_header *)p_read(p, fd, sizeof(*ofph));
	if (ofph->version != OFP_VERSION)
		errx(1, "invalid packet version: was %d, should be %d", ofph->version, OFP_VERSION);
	len = ntohs(ofph->length);
	p_read(p, fd, len - sizeof(*ofph));

	return (p);
}

void
ofproto_handle(struct ofswitch *ofs, int fd)
{
	struct packet *p;
	const struct ofp_header *ofph;

	p = ofproto_read(fd);
	ofph = (const struct ofp_header *)p->p_payload;
	debug("fd %d: received %s packet (%d bytes) from the %s\n",
	    fd, oftype_str(ofph->type), ntohs(ofph->length),
	    fd == ofs->ofs_controller_fd ? "controller" : "switch");

	switch (ofph->type) {
	case OFPT_FEATURES_REPLY:
	case OFPT_PORT_STATUS:
	case OFPT_PORT_MOD:
	case OFPT_STATS_REPLY:
		monitoring_handle(ofs, p);
		break;
	case OFPT_PACKET_IN:
	case OFPT_PACKET_OUT:
#ifdef notyet
		topology_handle(ofs, p);
#endif
		break;
	}

	if (fd == ofs->ofs_controller_fd)
		p_write(p, ofs->ofs_switch_fd);
	else
		p_write(p, ofs->ofs_controller_fd);
	p_free(p);
}
