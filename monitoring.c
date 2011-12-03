#include <err.h>
#include "monitoring.h"
#include "ofswitch.h"
#include "openflow.h"
#include "packet.h"

void
monitoring_handle(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_header *ofph;

	ofph = (const struct ofp_header *)p->p_payload;

	switch (ofph->type) {
	case OFPT_FEATURES_REPLY:
	case OFPT_PORT_STATUS:
	case OFPT_PORT_MOD:
	case OFPT_STATS_REPLY:
	default:
		errx(1, "monitoring_handle: no idea how to handle packet type %d", ofph->type);
	}
}

