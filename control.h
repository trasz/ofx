#ifndef CONTROL_H
#define	CONTROL_H

#include <stdint.h>

struct ofport;

void	control_port_up(struct ofport *ofp);
void	control_port_down(struct ofport *ofp);
void	control_port_advertise(struct ofport *ofp, uint32_t advertise);

#endif /* !CONTROL_H */
