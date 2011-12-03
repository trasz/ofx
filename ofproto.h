#ifndef OFPROTO_H
#define	OFPROTO_H

struct ofswitch;

void	ofproto_handle(struct ofswitch *ofs, int fd);

#endif /* !OFPROTO_H */
