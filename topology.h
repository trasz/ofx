#ifndef TOPOLOGY_H
#define	TOPOLOGY_H

struct ofswitch;
struct packet;

void	topology_handle_packet_in(struct ofswitch *ofs, const struct packet *p);
void	topology_handle_packet_out(struct ofswitch *ofs, const struct packet *p);

#endif /* !TOPOLOGY_H */
