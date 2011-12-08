#ifndef MONITORING_H
#define	MONITORING_H

struct ofport;
struct ofswitch;
struct packet;

void	mon_handle_features_reply(struct ofswitch *ofs, const struct packet *p);
void	mon_handle_port_status(struct ofswitch *ofs, const struct packet *p);
void	mon_handle_port_mod(struct ofswitch *ofs, const struct packet *p);
void	mon_handle_stats_reply(struct ofswitch *ofs, const struct packet *p);
void	mon_stats_request(struct ofport *ofp);

#endif /* !MONITORING_H */

