#include <sys/queue.h>
#include <netinet/in.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "openflow.h"
#include "ofproto.h"
#include "ofswitch.h"
#include "packet.h"

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
	debug("received packet type %d (%d bytes) from the %s\n",
	    ofph->type, ntohs(ofph->length),
	    fd == ofs->ofs_controller_fd ? "controller" : "switch");

	if (fd == ofs->ofs_controller_fd)
		p_write(p, ofs->ofs_switch_fd);
	else
		p_write(p, ofs->ofs_controller_fd);
	p_free(p);
}
