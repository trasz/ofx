#define _GNU_SOURCE /* For asprintf(3) under Linux. */
#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

struct array {
	int	a_type;
	int	a_count;
	int	a_allocated_count;
	union {
		int	*av_ints;
		double	*av_doubles;
		char	**av_strings;
	} a_values;
};

#define	A_TYPE_NONE	0
#define	A_TYPE_INT	1
#define	A_TYPE_DOUBLE	2
#define	A_TYPE_STRING	3

struct array *
a_alloc(void)
{
	struct array *a;

	a = calloc(sizeof(*a), 1);
	if (a == NULL)
		err(1, "calloc");

	return (a);
}

void
a_free(struct array *a)
{
	int i;

	if (a->a_type == A_TYPE_STRING)
		for (i = 0; i < a->a_count; i++)
			free(a->a_values.av_strings[i]);

	free(a->a_values.av_ints);
	memset(a, 0, sizeof(*a));
	free(a);
}

static void
a_extend(struct array *a)
{

	a->a_count++;

	if (a->a_count < a->a_allocated_count)
		return;

	a->a_allocated_count *= 4;
	if (a->a_count  >= a->a_allocated_count)
		a->a_allocated_count = a->a_count;

	switch (a->a_type) {
	case A_TYPE_INT:
		a->a_values.av_ints = realloc(a->a_values.av_ints, a->a_allocated_count * sizeof(int));
		break;
	case A_TYPE_DOUBLE:
		a->a_values.av_doubles = realloc(a->a_values.av_doubles, a->a_allocated_count * sizeof(double));
		break;
	case A_TYPE_STRING:
		a->a_values.av_strings = realloc(a->a_values.av_strings, a->a_allocated_count * sizeof(char *));
		break;
	default:
		assert(!"invalid array type");
	}

	if (a->a_values.av_ints == NULL)
		err(1, "realloc");
}

void
a_add_int(struct array *a, int n)
{

	assert(a->a_type == A_TYPE_INT || a->a_type == A_TYPE_NONE);
	a->a_type = A_TYPE_INT;

	a_extend(a);
	a->a_values.av_ints[a->a_count - 1] = n;
}

void
a_add_double(struct array *a, double x)
{

	assert(a->a_type == A_TYPE_DOUBLE || a->a_type == A_TYPE_NONE);
	a->a_type = A_TYPE_DOUBLE;

	a_extend(a);
	a->a_values.av_doubles[a->a_count - 1] = x;
}

void
a_add_string(struct array *a, const char *s)
{

	assert(a->a_type == A_TYPE_STRING || a->a_type == A_TYPE_NONE);
	a->a_type = A_TYPE_STRING;

	a_extend(a);
	a->a_values.av_strings[a->a_count - 1] = strdup(s);
	if (a->a_values.av_strings[a->a_count - 1] == NULL)
		err(1, "malloc");
}

char *
a_str(struct array *a)
{
	int i;
	char *str, *str2;

	str = strdup("[");

	for (i = 0; i < a->a_count; i++) {
		switch (a->a_type) {
		case A_TYPE_INT:
			asprintf(&str2, "%s%d", str, a->a_values.av_ints[i]);
			free(str);
			break;
		case A_TYPE_DOUBLE:
			asprintf(&str2, "%s%f", str, a->a_values.av_doubles[i]);
			free(str);
			break;
		case A_TYPE_STRING:
			asprintf(&str2, "%s\"%s\"", str, a->a_values.av_strings[i]);
			free(str);
			break;
		default:
			assert(!"invalid array type");
		}

		if (i < a->a_count - 1) {
			asprintf(&str, "%s, ", str2);
			free(str2);
		} else
			str = str2;
	}

	asprintf(&str2, "%s]\n", str);
	free(str);

	return(str2);
}

