#include <netinet/in.h>
#include <err.h>
#include <string.h>

#include "control.h"
#include "debug.h"
#include "ofport.h"
#include "ofswitch.h"
#include "openflow.h"
#include "packet.h"

static void
control_port_mod(struct ofport *ofp, uint32_t config, uint32_t mask)
{
	struct packet *p;
	struct ofp_port_mod *ofppm;

	p = p_alloc();
	ofppm = (struct ofp_port_mod *)p_extend(p, sizeof(*ofppm));
	ofppm->header.version = OFP_VERSION;
	ofppm->header.type = OFPT_PORT_MOD;
	ofppm->header.length = htons(p->p_payload_len);
	ofppm->header.xid = 0; /* XXX */

	ofppm->port_no = htons(ofp->ofp_number);
	memcpy(ofppm->hw_addr, ofp->ofp_hw_addr, sizeof(ofp->ofp_hw_addr));
	ofppm->config = htonl(config);
	ofppm->mask = htonl(mask);
	ofppm->advertise = htonl(ofp->ofp_advertised);

	p_write(p, ofp->ofp_switch->ofs_switch_fd);
	p_free(p);
}

void
control_port_up(struct ofport *ofp)
{

	control_port_mod(ofp, 0, OFPPC_PORT_DOWN);
}

void
control_port_down(struct ofport *ofp)
{

	control_port_mod(ofp, OFPPC_PORT_DOWN, OFPPC_PORT_DOWN);
}

