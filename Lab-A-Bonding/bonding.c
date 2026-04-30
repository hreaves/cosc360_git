#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bonding.h"
#include "dllist.h"

/* This solution compiles and hangs. */

typedef struct molecule{

	Dllist hydrogen;
	Dllist oxygen;
	pthread_mutex_t *lock;

} Molecule;

void *initialize_v(char *verbosity) {
	Molecule *m;
	m = (Molecule *) malloc(sizeof(Molecule));
	m->hydrogen = new_dllist();
	m->oxygen = new_dllist();
	m->lock = new_mutex();
	return (void *) m;
}
// arg is int id and void *v
void *hydrogen(void *arg) {
	Molecule *h;
	h = initialize_v();

	return NULL;
}

void *oxygen(void *arg) {
	Molecule *o;
	o = initialize();

	return NULL;
}
