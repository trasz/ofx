#ifndef MONITORING_H
#define	MONITORING_H

struct ofswitch;
struct packet;

void	monitoring_handle(struct ofswitch *ofs, const struct packet *p);

#endif /* !MONITORING_H */

