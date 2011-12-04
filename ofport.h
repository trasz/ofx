#ifndef  OFPORT_H
#define	OFPORT_H

#include <sys/types.h>
#include <sys/queue.h>

#include "openflow.h"

struct ofswitch;
struct oflink;

struct ofport {
	TAILQ_ENTRY(ofport)	ofp_next;
	struct ofswitch		*ofp_switch;
	struct oflink		*ofp_link;
	TAILQ_ENTRY(ofport)	ofp_next_link;
	int			ofp_number;
	uint8_t			ofp_hw_addr[OFP_ETH_ALEN];
	char			*ofp_name;
	uint32_t		ofp_config;
	uint32_t		ofp_state;
	uint32_t		ofp_curr;
	uint32_t		ofp_advertised;
	uint32_t		ofp_supported;
	uint32_t		ofp_peer;
};

struct ofport	*ofp_alloc(void);
void		ofp_free(struct ofport *ofp);
struct ofport	*ofp_find_by_number(struct ofswitch *ofs, int number);

#endif /* !OFPORT_H */
