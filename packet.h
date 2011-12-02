#ifndef PACKET_H
#define	PACKET_H

struct packet {
	unsigned char		*p_payload;
	size_t			p_payload_len;
	size_t			p_allocated_len;
};

struct packet	*p_alloc(void);
void		p_free(struct packet *p);
unsigned char	*p_extend(struct packet *p, size_t diff);
unsigned char	*p_read(struct packet *p, int fd, size_t len);
unsigned char	*p_peek(struct packet *p, int fd, size_t len);
void		p_write(struct packet *p, int fd);
void		p_dump(struct packet *p);

#endif /* !PACKET_H */
