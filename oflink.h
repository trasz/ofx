#ifndef OFLINK_H
#define	OFLINK_H

#include <sys/queue.h>

struct oflink {
	TAILQ_HEAD(, ofport)	ofl_ports;
};

void	ofl_link(struct ofport *ofp1, struct ofport *ofp2);
void	ofl_unlink(struct ofport *ofp);

#endif /* !OFLINK_H */
