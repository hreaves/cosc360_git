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

void check_name(Person *p, JRB tree, IS is) {
	char temp_name[100];
	strcpy(tmp_name, is->fields[1]);
    for (i = 1; i < is->NF; i++) {
		strcat(tmp_name, " ");
        strcat(tmp->name, is->fields[i]);
	}
	tmp = jrb_find_str(tree, tmp_name);
    if(tmp==NULL) { // name doesnt exist
		p = (Person *) malloc(size(Person));
		jrb_insert_str(tree, p->name, p);
		p->name = (char *) malloc(100);
		strcpy(p->name, tmp_name);
	}
	else {
		curr_person = (JRB) tmp->val.v;
	}
}

int main(int argc, char **arv) {
	
	IS is;
	Jval jv;
	JRB tree, tmp;

	int i;

	is = new_inputstruct(argv[1]);
	if (is == NULL) {
		perror(argv[1]);
		exit(1);
	}
	Person *curr_person;
	Person *relative;
	while(get_line(is) >= 0) {

		if (strcmp(is->fields[0], "PERSON")==0) {
			check_name(curr_person, tree, is);
		}
		else if (strcmp(is->fields[0], "FATHER")==0) {
			check_name(relative, tree, is);
			strcpy(relative->children, curr_person->name);
			strcpy(curr_person->father, relative->name);
		}
		else if (strcmp(is->fields[0], "MOTHER")==0) {
			check_name(relative, tree, is);
			strcpy(relative->children, curr_person->name);
			strcpy(curr_person->mother, relative->name);
		}
		else if (strcmp(is->fields[0], "FATHER_OF")==0) {
            check_name(relative, tree, is);
            strcpy(relative->father, curr_person->name);
			strcpy(curr_person->children, relative->name);
        }
		else if (strcmp(is->fields[0], "MOTHER_OF")==0) {
			check_name(relative, tree, is);
			strcpy(relative->mother, curr_person->name);
			strcpy(curr_person->children, relative->name);
		}
		else if (strcmp(is->fields[0], "SEX")==0) {
			strcpy(curr_person->sex, is->fields[1]);
		}
	}

	jettison_inputstruct(is);



	return 0;
}
