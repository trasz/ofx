#include <netinet/in.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "ofport.h"
#include "ofswitch.h"
#include "openflow.h"
#include "packet.h"
#include "topology.h"

struct topology_packet {
	TAILQ_ENTRY(topology_packet)	tp_next;
	struct ofport			*tp_port;
	struct packet			*tp_p;
};

static TAILQ_HEAD(, topology_packet)		topology_packets;

static struct topology_packet *
tp_alloc(void)
{
	struct topology_packet *tp;

	tp = calloc(sizeof(*tp), 1);
	if (tp == NULL)
		err(1, "calloc");

	tp->tp_p = p_alloc();

	return (tp);
}

void
topology_handle_packet_in(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_packet_in *ofppi;
	struct topology_packet *tp;
	int frame_len, compare_len;

	if (p->p_payload_len < sizeof(*ofppi))
		errx(1, "invalid PACKET_IN message size: got %zd, should be at least %ld", p->p_payload_len, sizeof(*ofppi));

	frame_len = p->p_payload_len - sizeof(*ofppi);

	TAILQ_FOREACH(tp, &topology_packets, tp_next) {
		compare_len = tp->tp_p->p_payload_len;
		if (frame_len < compare_len)
			compare_len = frame_len;
		debug("topology_handle_packet_in: comparing %d bytes\n", compare_len);
		if (memcmp(ofppi->data, tp->tp_p->p_payload, compare_len) != 0)
			continue;

		debug("topology_handle_packet_in: got link from switch %d port %d to switch %d port %d\n",
		    tp->tp_port->ofp_switch->ofs_number, tp->tp_port->ofp_number, ofs->ofs_number, ntohs(ofppi->in_port));
	}
}

void
topology_handle_packet_out(struct ofswitch *ofs, const struct packet *p)
{
	const struct ofp_packet_out *ofppo;
	struct topology_packet *tp;
	int frame_len;

	if (p->p_payload_len < sizeof(*ofppo))
		errx(1, "invalid PACKET_IN message size: got %zd, should be at least %ld", p->p_payload_len, sizeof(*ofppo));

	/*
	 * XXX: Zamiast przechowywać i porównywać cały pakiet moglibyśmy liczyć sumę.
	 */
	tp = tp_alloc();
	frame_len = p->p_payload_len - sizeof(*ofppo);
	p_extend(tp->tp_p, frame_len);
	memcpy(tp->tp_p->p_payload, ofppo->data, frame_len);

	tp->tp_port = ofp_find_by_number(ofs, ntohs(ofppo->in_port));

	TAILQ_INSERT_TAIL(&topology_packets, tp, tp_next);

	/*
	 * XXX: Usuwać tp po jakimś czasie.
	 */
}

