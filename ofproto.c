#include <sys/queue.h>
#include <netinet/in.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "openflow.h"
#include "ofproto.h"
#include "packet.h"

struct ofswitch {
	TAILQ_ENTRY(ofswitch)	ofs_next;
	int			ofs_switch_fd;
	int			ofs_controller_fd;
};

TAILQ_HEAD(, ofswitch)		ofswitches;

void
ofproto_new_switch(int switch_fd, int controller_fd)
{
	struct ofswitch *ofs;

	ofs = calloc(sizeof(*ofs), 1);
	if (ofs == NULL)
		err(1, "calloc");

	ofs->ofs_switch_fd = switch_fd;
	ofs->ofs_controller_fd = controller_fd;

	TAILQ_INSERT_TAIL(&ofswitches, ofs, ofs_next);
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

struct ofswitch *
ofproto_find_switch(int fd)
{
	struct ofswitch *ofs;

	TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
		if (fd == ofs->ofs_switch_fd || fd == ofs->ofs_controller_fd)
			return (ofs);
	}

	return (NULL);
}

bool
ofproto_handle(int fd)
{
	struct packet *p;
	const struct ofp_header *ofph;
	struct ofswitch *ofs;

	ofs = ofproto_find_switch(fd);
	if (ofs == NULL)
		return (false);

	p = ofproto_read(fd);
	ofph = (const struct ofp_header *)p->p_payload;
	printf("got packet type %d\n", ofph->type);

	if (fd == ofs->ofs_controller_fd)
		p_write(p, ofs->ofs_switch_fd);
	else
		p_write(p, ofs->ofs_controller_fd);
	p_free(p);

	return (true);
}
