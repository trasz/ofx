#define _GNU_SOURCE /* For asprintf(3) under Linux. */
#include <sys/endian.h>
#include <netinet/in.h>
#include <err.h>
#include <string.h>

#include "debug.h"
#include "monitoring.h"
#include "ofport.h"
#include "ofswitch.h"
#include "openflow.h"
#include "packet.h"

#define	ntohq(x)	be64toh(x);

static struct ofport *
mon_parse_ofp_phy_port(const struct ofp_phy_port *ofppp)
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
mon_handle_features_reply(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_switch_features *ofpsf;
	const struct ofp_phy_port *ofppp;
	int i, nports;
	size_t expected_len;
	struct ofport *ofp;

	if (p->p_payload_len < sizeof(*ofpsf))
		errx(1, "invalid FEATURES_REPLY message size: got %zd, should be at least %zd", p->p_payload_len, sizeof(*ofpsf));

	ofpsf = (const struct ofp_switch_features *)p->p_payload;

	nports = (p->p_payload_len - sizeof(*ofpsf)) / sizeof(struct ofp_phy_port);
	expected_len = sizeof(*ofpsf) + nports * sizeof(struct ofp_phy_port);
	if (p->p_payload_len != expected_len)
		errx(1, "invalid FEATURES_REPLY message size: got %zd, for %d ports should be %zd", p->p_payload_len, nports, expected_len);

	ofs->ofs_datapath_id = ntohq(ofpsf->datapath_id);
	ofs->ofs_nports = nports;
	debug("controller %d: %d ports", ofs->ofs_number, ofs->ofs_nports);

	for (i = 0; i < nports; i++) {
		ofppp = (const struct ofp_phy_port *)&(ofpsf->ports[i]);
		ofp = mon_parse_ofp_phy_port(ofppp);
		ofs_add_port(ofs, ofp);
	}
}

void
mon_handle_port_status(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_port_status *ofpps;
	struct ofport *ofp;

	if (p->p_payload_len != sizeof(*ofpps))
		errx(1, "invalid PORT_STATUS message size: got %zd, should be %zd", p->p_payload_len, sizeof(*ofpps));

	ofpps = (const struct ofp_port_status *)p->p_payload;

	ofp = mon_parse_ofp_phy_port(&(ofpps->desc));
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
mon_handle_port_mod(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_port_mod *ofppm;
	struct ofport *ofp;

	if (p->p_payload_len != sizeof(*ofppm))
		errx(1, "invalid PORT_MOD message size: got %zd, should be %zd", p->p_payload_len, sizeof(*ofppm));

	ofppm = (const struct ofp_port_mod *)p->p_payload;

	ofp = ofp_find_by_number(ofs, ntohs(ofppm->port_no));
	if (ofp == NULL)
		errx(1, "port not found");
	ofp->ofp_config &= ~(ntohl(ofppm->mask));
	ofp->ofp_config |= ntohl(ofppm->config);
	ofp->ofp_advertised = ntohl(ofppm->advertise);
}

void
mon_stats_request(struct ofport *ofp)
{
	struct packet *p;
	struct ofp_stats_msg *ofpsm;

	p = p_alloc();
	ofpsm = (struct ofp_stats_msg *)p_extend(p, sizeof(*ofpsm));
	ofpsm->header.version = OFP_VERSION;
	ofpsm->header.type = OFPT_STATS_REQUEST;
	ofpsm->header.length = htons(p->p_payload_len);
	ofpsm->header.xid = 0; /* XXX */

	ofpsm->type = htons(OFPST_PORT);
	ofpsm->flags = htons(0);

	p_write(p, ofp->ofp_switch->ofs_switch_fd);
	p_free(p);
}

/*
 * Tej struktury z jakiegoś powodu nie ma w specifykacji.
 */
struct ofp_port_stats_reply {
	struct ofp_stats_msg osm;
	struct ofp_port_stats ports[];
};

void
mon_handle_stats_reply(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_port_stats_reply *ofppsr;
	const struct ofp_port_stats *ofpps;
	struct ofport *ofp;
	int i, nports;
	size_t expected_len;

	if (p->p_payload_len < sizeof(*ofppsr))
		errx(1, "invalid STATS_REPLY message size: got %zd, should be at least %zd", p->p_payload_len, sizeof(*ofppsr));

	ofppsr  = (const struct ofp_port_stats_reply *)p->p_payload;
	if (ntohs(ofppsr->osm.type) != OFPST_PORT)
		return;

	nports = (p->p_payload_len - sizeof(*ofppsr)) / sizeof(struct ofp_port_stats);
	expected_len = sizeof(*ofppsr) + nports * sizeof(struct ofp_port_stats);
	if (p->p_payload_len != expected_len)
		errx(1, "invalid STATS_REPLY message size: got %zd, for %d ports should be %zd", p->p_payload_len, nports, expected_len);

	for (i = 0; i < nports; i++) {
		ofpps = (const struct ofp_port_stats *)&(ofppsr->ports[i]);

		ofp = ofp_find_by_number(ofs, ntohs(ofpps->port_no));
		if (ofp == NULL)
			errx(1, "port not found");

		ofp->ofp_rx_packets = ntohq(ofpps->rx_packets);
		ofp->ofp_tx_packets = ntohq(ofpps->tx_packets);
		ofp->ofp_rx_bytes = ntohq(ofpps->rx_bytes);
		ofp->ofp_tx_bytes = ntohq(ofpps->tx_bytes);
		ofp->ofp_rx_dropped = ntohq(ofpps->rx_dropped);
		ofp->ofp_tx_dropped = ntohq(ofpps->tx_dropped);
		ofp->ofp_rx_errors = ntohq(ofpps->rx_errors);
		ofp->ofp_tx_errors = ntohq(ofpps->tx_errors);
		ofp->ofp_rx_frame_err = ntohq(ofpps->rx_frame_err);
		ofp->ofp_rx_over_err = ntohq(ofpps->rx_over_err);
		ofp->ofp_rx_crc_err = ntohq(ofpps->rx_crc_err);
		ofp->ofp_collisions = ntohq(ofpps->collisions);
	}
}

