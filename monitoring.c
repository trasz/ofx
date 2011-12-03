#include <err.h>

#include "debug.h"
#include "monitoring.h"
#include "ofswitch.h"
#include "openflow.h"
#include "packet.h"

void
monitoring_handle_features_reply(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_switch_features *ofpsf;
	int nports;
	size_t expected_len;

	if (p->p_payload_len < sizeof(*ofpsf))
		errx(1, "invalid FEATURES_REPLY message size: got %zd, should be at least %ld", p->p_payload_len, sizeof(*ofpsf));

	nports = (p->p_payload_len - sizeof(*ofpsf)) / sizeof(struct ofp_phy_port);
	expected_len = sizeof(*ofpsf) + nports * sizeof(struct ofp_phy_port);
	if (p->p_payload_len != expected_len)
		errx(1, "invalid FEATURES_REPLY message size: got %zd, for %d ports should be %zd", p->p_payload_len, nports, expected_len);

	ofs->ofs_nports = nports;
	debug("controller %d: %d ports", ofs->ofs_number, ofs->ofs_nports);
}

void
monitoring_handle_port_status(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_port_status *ofpps;

	if (p->p_payload_len != sizeof(*ofpps))
		errx(1, "invalid PORT_STATUS message size: got %zd, should be %ld", p->p_payload_len, sizeof(*ofpps));

	ofpps = (const struct ofp_port_status *)p->p_payload;
	switch (ofpps->reason) {
	case OFPPR_ADD:
		debug("switch %d: port added\n", ofs->ofs_number);
		break;
	case OFPPR_DELETE:
		debug("switch %d: port deleted\n", ofs->ofs_number);
	case OFPPR_MODIFY:
		debug("switch %d: port changed\n", ofs->ofs_number);
	default:
		errx(1, "invalid port status reason %d", ofpps->reason);
	}
}

void
monitoring_handle_port_mod(struct ofswitch *ofs, const struct packet *p)
{
}

void
monitoring_handle_stats_reply(struct ofswitch *ofs, const struct packet *p)
{
}

