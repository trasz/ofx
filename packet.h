#ifndef PACKET_H
#define	PACKET_H

#include <sys/types.h>

struct packet {
	char			*p_payload;
	size_t			p_payload_len;
	size_t			p_allocated_len;
};

struct packet	*p_alloc(void);
void		p_free(struct packet *p);
char		*p_extend(struct packet *p, size_t diff);
char		*p_read(struct packet *p, int fd, size_t len);
char		*p_peek(struct packet *p, int fd, size_t len);
void		p_write(struct packet *p, int fd);

#endif /* !PACKET_H */
