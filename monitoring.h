#ifndef MONITORING_H
#define	MONITORING_H

struct ofswitch;
struct packet;

void	monitoring_handle_features_reply(struct ofswitch *ofs, const struct packet *p);
void	monitoring_handle_port_status(struct ofswitch *ofs, const struct packet *p);
void	monitoring_handle_port_mod(struct ofswitch *ofs, const struct packet *p);
void	monitoring_handle_stats_reply(struct ofswitch *ofs, const struct packet *p);

#endif /* !MONITORING_H */

