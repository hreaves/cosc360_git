// COSC360 Lab2 Libfdr Primer
// Harrison Reaves
//
#include <stdio.h>
#include <stdlib.h>
#include "fields.h"
#include "jval.h"
#include "jrb.h"

typedef struct person {
	char *name; // point to array[0] since size unknown
	char *sex; // Male, Female, or Unknown
	char *father; // Name or Unknown 
	char *mother; // Name or Unknown
	char *children; // Names or None
} Person;

main(int argc, char **arv) {
	
	IS is;
	Jval jv;
	

	return 0;
}
