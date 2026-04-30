/*  COSC360 LabA The Bonding Lab
	Harrison Reaves
	This program is called by bonding-driver to check and wait until two hydrogens and a one oxygen molecule
	can be bonded together, and once they can be, to have them all three pass the same ids in the same order to
	Bond() them and get their shared string.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bonding.h"
#include "dllist.h"
// Lists of waiting oxygen and hydrogen molecules and shared mutex lock
typedef struct global{
	Dllist hydrogens;
	Dllist oxygens;
	pthread_mutex_t *lock;
} Global;
// Info for each individual molecule
typedef struct molecule{
	int id;
	int h1;
	int h2;
	int o;
	pthread_cond_t *cond;
} Molecule;
// Creates the struct holding the dllists and lock, given to hydrogen() and oxygen()
void *initialize_v(char *verbosity) {
	Global *g;
	g = (Global *) malloc(sizeof(Global));
	g->hydrogens = new_dllist();
	g->oxygens = new_dllist();
	g->lock = new_mutex();
	return (void *) g;
}
// arg is int id (of thread) and void *v (Global created in initialize_v)
void *hydrogen(void *arg) {
	
	struct bonding_arg *b;
	Global *g;
	Molecule *h1, *h2, *o;
	char *rv;
	
	b = (struct bonding_arg *) arg;
	g = (Global *) b->v; // the global struct
	// create molecule and initalize fields
	h1 = (Molecule *) malloc(sizeof(Molecule));
	h1->id = b->id;
	h1->h1 = -1;
	h1->h2= -1;
	h1->o = -1;
	h1->cond = new_cond();
	pthread_mutex_lock(g->lock); // lock the mutex before checking the lists
	// If there is a hydrogen and oxygen molecule available, start the process of bonding them with the current hydrogen
	if(!dll_empty(g->oxygens) && !dll_empty(g->hydrogens)) {
		Dllist tmp1, tmp2;
		// get the oxygen and hydrogen nodes from the front of the dllist
        tmp1 = g->hydrogens->flink;
		tmp2 = g->oxygens->flink;
		h2 = tmp1->val.v; // tmp->val.v will be address to the molecule struct
		o = tmp2->val.v;
		// set the all the ids of the molecules to be bonded so that they match eachother when Bond() is called
		h1->h1 = h1->id;
		h1->h2 = h2->id;
		h1->o = o->id;
		
		h2->h1 = h1->id;
		h2->h2 = h2->id;
		h2->o = o->id;

		o->h1 = h1->id;
		o->h2 = h2->id;
		o->o = o->id;
		// remove their nodes from the dllist since they're no longer waiting
		dll_delete_node(tmp1);
        dll_delete_node(tmp2);
		// signal the other two molecule's condition to wake them up, release the mutex, and call bond
		pthread_cond_signal(h2->cond);
		pthread_cond_signal(o->cond);
		pthread_mutex_unlock(g->lock);
		rv = Bond(h1->h1, h1->h2, h1->o);
	}
	// If not enough molecules to create bond, add to the list and wait for condition to be released
	else {
		dll_append(g->hydrogens, new_jval_v(h1));
		pthread_cond_wait(h1->cond, g->lock);
		// Once released, unlock the mutex and call bond in same order as other molecules
		pthread_mutex_unlock(g->lock);
		rv = Bond(h1->h1, h1->h2, h1->o);
	}
	// clean up
	pthread_cond_destroy(h1->cond);	
	free(h1->cond);
	free(h1);
	return (void *) rv;
}
// Mostly same as hydrogen function except you check for the first two nodes in the hydrogen dllist
void *oxygen(void *arg) {
    struct bonding_arg *b;
    Global *g;
    Molecule *h1, *h2, *o;
    char *rv;

    b = (struct bonding_arg *) arg;
    g = (Global *) b->v;
    o = (Molecule *) malloc(sizeof(Molecule));
	o->id = b->id;
	o->h1 = -1;
	o->h2 = -1;
	o->o = -1;
	o->cond = new_cond();

    pthread_mutex_lock(g->lock);
    if(!dll_empty(g->hydrogens) && g->hydrogens->flink->flink != g->hydrogens) {
        Dllist tmp1, tmp2;
		tmp1 = g->hydrogens->flink;
		tmp2 = g->hydrogens->flink->flink;
		h1 = tmp1->val.v;
        h2 = tmp2->val.v;
        h1->h1 = h1->id;
        h1->h2 = h2->id;
        h1->o = o->id;

        h2->h1 = h1->id;
        h2->h2 = h2->id;
        h2->o = o->id;

        o->h1 = h1->id;
        o->h2 = h2->id;
        o->o = o->id;

        dll_delete_node(tmp1);
        dll_delete_node(tmp2);

        pthread_cond_signal(h1->cond);
        pthread_cond_signal(h2->cond);
        pthread_mutex_unlock(g->lock);
        rv = Bond(o->h1, o->h2, o->o);
    }
    else {
        dll_append(g->oxygens, new_jval_v(o));
        pthread_cond_wait(o->cond, g->lock);
        pthread_mutex_unlock(g->lock);
        rv = Bond(o->h1, o->h2, o->o);
    }
	
	pthread_cond_destroy(o->cond);
    free(o->cond);
    free(o);
    return (void *) rv;
}
