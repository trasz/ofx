#ifndef CONTROL_H
#define	CONTROL_H

struct ofport;

void	control_port_up(struct ofport *ofp);
void	control_port_down(struct ofport *ofp);

#endif /* !CONTROL_H */
