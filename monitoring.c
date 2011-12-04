#include <netinet/in.h>
#include <err.h>
#include <string.h>

#include "debug.h"
#include "monitoring.h"
#include "ofport.h"
#include "ofswitch.h"
#include "openflow.h"
#include "packet.h"

static struct ofport *
monitoring_parse_ofp_phy_port(const struct ofp_phy_port *ofppp)
{
	struct ofport *ofp;

	ofp = ofp_alloc();
	ofp->ofp_number = ntohs(ofppp->port_no);
	memcpy(ofp->ofp_hw_addr, ofppp->hw_addr, sizeof(ofp->ofp_hw_addr));
	ofp->ofp_name = strndup(ofppp->name, OFP_MAX_PORT_NAME_LEN);
	ofp->ofp_config = ntohl(ofppp->config);
	ofp->ofp_state = ntohl(ofppp->state);
	ofp->ofp_curr = ntohl(ofppp->curr);
	ofp->ofp_advertised = ntohl(ofppp->advertised);
	ofp->ofp_supported = ntohl(ofppp->supported);
	ofp->ofp_peer = ntohl(ofppp->peer);

	return (ofp);
}

void
monitoring_handle_features_reply(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_switch_features *ofpsf;
	const struct ofp_phy_port *ofppp;
	int i, nports;
	size_t expected_len;
	struct ofport *ofp;

	if (p->p_payload_len < sizeof(*ofpsf))
		errx(1, "invalid FEATURES_REPLY message size: got %zd, should be at least %ld", p->p_payload_len, sizeof(*ofpsf));

	nports = (p->p_payload_len - sizeof(*ofpsf)) / sizeof(struct ofp_phy_port);
	expected_len = sizeof(*ofpsf) + nports * sizeof(struct ofp_phy_port);
	if (p->p_payload_len != expected_len)
		errx(1, "invalid FEATURES_REPLY message size: got %zd, for %d ports should be %zd", p->p_payload_len, nports, expected_len);

	ofs->ofs_nports = nports;
	debug("controller %d: %d ports", ofs->ofs_number, ofs->ofs_nports);

	for (i = 0; i < nports; i++) {
		ofppp = (const struct ofp_phy_port *)&(ofpsf->ports[i]);
		ofp = monitoring_parse_ofp_phy_port(ofppp);
		ofs_add_port(ofs, ofp);
	}
}

void
monitoring_handle_port_status(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_port_status *ofpps;
	struct ofport *ofp;

	if (p->p_payload_len != sizeof(*ofpps))
		errx(1, "invalid PORT_STATUS message size: got %zd, should be %ld", p->p_payload_len, sizeof(*ofpps));

	ofpps = (const struct ofp_port_status *)p->p_payload;
	ofp = monitoring_parse_ofp_phy_port(&(ofpps->desc));
	switch (ofpps->reason) {
	case OFPPR_ADD:
		debug("switch %d: port added\n", ofs->ofs_number);
		ofs_add_port(ofs, ofp);
		break;
	case OFPPR_DELETE:
		debug("switch %d: port deleted\n", ofs->ofs_number);
		ofs_delete_port(ofs, ofp);
		break;
	case OFPPR_MODIFY:
		debug("switch %d: port changed\n", ofs->ofs_number);
		ofs_modify_port(ofs, ofp);
		break;
	default:
		errx(1, "invalid port status reason %d", ofpps->reason);
	}
}

void
monitoring_handle_port_mod(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_port_mod *ofppm;
	struct ofport *ofp;

	if (p->p_payload_len != sizeof(*ofppm))
		errx(1, "invalid PORT_MOD message size: got %zd, should be %ld", p->p_payload_len, sizeof(*ofppm));

	ofp = ofp_find_by_number(ofs, ntohs(ofppm->port_no));
	if (ofp == NULL)
		errx(1, "port not found");
	ofp->ofp_config &= ~(ntohl(ofppm->mask));
	ofp->ofp_config |= ntohl(ofppm->config);
	ofp->ofp_advertised = ntohl(ofppm->advertise);
}

void
monitoring_handle_stats_reply(struct ofswitch *ofs, const struct packet *p)
{
}

