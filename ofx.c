#include <stdio.h>

#include "openflow.h"

#include "array.h"
#include "packet.h"

int
main(int argc, char **argv)
{
	struct packet *p;
	struct array *a;

	p = p_alloc();
	p_free(p);

	a = a_alloc();
	a_add_string(a, "ala");
	a_add_string(a, "ma");
	a_add_string(a, "kota");
	a_add_string(a, "alika");
	a_str(stdout, a);
	a_free(a);

	return (0);
}

