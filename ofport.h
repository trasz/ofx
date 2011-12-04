#ifndef  OFPORT_H
#define	OFPORT_H

#include <sys/types.h>
#include <sys/queue.h>

struct ofport {
	TAILQ_ENTRY(ofport)	ofp_next;
	struct ofswitch		*ofp_switch;
	int			ofp_number;
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

#endif /* !OFPORT_H */
