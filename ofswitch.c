#include <assert.h>
#include <err.h>
#include <stdlib.h>

#include "ofport.h"
#include "ofswitch.h"

struct ofswitches ofswitches;
static int ofswitch_last_number;

void
ofs_add(int switch_fd, int controller_fd)
{
	struct ofswitch *ofs;

	ofs = calloc(sizeof(*ofs), 1);
	if (ofs == NULL)
		err(1, "calloc");

	ofs->ofs_switch_fd = switch_fd;
	ofs->ofs_controller_fd = controller_fd;
	ofs->ofs_number = ofswitch_last_number;
	ofswitch_last_number++;

	TAILQ_INSERT_TAIL(&ofswitches, ofs, ofs_next);
}

struct ofswitch *
ofs_find(int fd)
{
	struct ofswitch *ofs;

	TAILQ_FOREACH(ofs, &ofswitches, ofs_next) {
		if (fd == ofs->ofs_switch_fd || fd == ofs->ofs_controller_fd)
			return (ofs);
	}

	return (NULL);
}

void
ofs_add_port(struct ofswitch *ofs, struct ofport *ofp)
{

	assert(ofp->ofp_switch == NULL);
	ofp->ofp_switch = ofs;
	TAILQ_INSERT_TAIL(&ofs->ofs_ports, ofp, ofp_next);
}

void
ofs_delete_port(struct ofswitch *ofs, struct ofport *ofp)
{
	struct ofport *tmp;

	tmp = ofs_find_port(ofs, ofp->ofp_number);
	TAILQ_REMOVE(&ofs->ofs_ports, tmp, ofp_next);
	ofp_free(tmp);
}

void
ofs_modify_port(struct ofswitch *ofs, struct ofport *ofp)
{
	struct ofport *tmp;

	tmp = ofs_find_port(ofs, ofp->ofp_number);

	tmp->ofp_config = ofp->ofp_config;
	tmp->ofp_state = ofp->ofp_state;
	tmp->ofp_curr = ofp->ofp_curr;
	tmp->ofp_advertised = ofp->ofp_advertised;
	tmp->ofp_supported = ofp->ofp_supported;
	tmp->ofp_peer = ofp->ofp_peer;
}

struct ofport *
ofs_find_port(struct ofswitch *ofs, int port)
{
	struct ofport *tmp;

	TAILQ_FOREACH(tmp, &ofs->ofs_ports, ofp_next) {
		assert(tmp->ofp_switch == ofs);
		if (tmp->ofp_number == port)
			return (tmp);
	}
	errx(1, "ofs_delete_port: port not found");
}

