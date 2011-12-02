#ifndef ARRAY_H
#define	ARRAY_H

#include <stdio.h>

struct array;

struct array	*a_alloc(void);
void		a_free(struct array *a);
void		a_add_int(struct array *a, int n);
void		a_add_double(struct array *a, double x);
void		a_add_string(struct array *a, const char *s);
void		a_str(FILE *fp, struct array *a);

#endif /* !ARRAY_H */
