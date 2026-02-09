// COSC360 Lab2 Libfdr Primer
// Harrison Reaves
/* This program takes a description of people and their relationships 
 * to one another on standard input, then prints out complete information
 * on all of the people in the file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fields.h"
#include "jrb.h"
#include "jval.h"
#include "dllist.h"

typedef struct person {
	char *name;
	char *sex; // Male, Female, or Unknown
	struct person *father; // Name or Unknown 
	struct person *mother; // Name or Unknown
	Dllist children; // Names or None
	int visiting;
	int visited;
	int printed;
} Person;
// Function to check if person already exists and returns their pointer
Person *name_check(JRB tree, IS is) {
	JRB tmp;
	Person *p;
	char tmp_name[100]; 
	// Creating the full name on this line
	strcpy(tmp_name, is->fields[1]);
    for (int i = 2; i < is->NF; i++) {
        strcat(tmp_name, " ");
        strcat(tmp_name, is->fields[i]);
    }
	// Searches for person in tree with their name as key
    tmp = jrb_find_str(tree, tmp_name);

    if(tmp==NULL) { // Name doesnt exist
        p = (Person *) malloc(sizeof(Person));
        p->name = strdup(tmp_name);
        p->sex = NULL;
        p->father = NULL;
        p->mother = NULL;
		p->children = new_dllist();
        p->visited = 0;
		p->visiting = 0;
		p->printed = 0;
		jrb_insert_str(tree, p->name, new_jval_v(p));
    }
    else {
        p = (Person*) tmp->val.v; // Pointer to existing person in tree
    }
	return p;
}
// Function to check for and create valid connections between two persons
void connection_check(Person *child, Person *parent, char *sex, int line) {
	if(strcmp(sex, "M")==0) { 
		if(child->father != NULL) {
			// Throw error if child has father and their names aren't the same
			if(child->father != parent) {
				fprintf(stderr, "Bad input -- child with two fathers on line %d\n", line);
				exit(1);
			}
		}
		else {
			// If child has no father, connect 
			child->father = parent;
		}
	}
	// Same but for mother to child
	else if (strcmp(sex, "F")==0) { 
		if(child->mother != NULL) {
			if(child->mother != parent) {
				fprintf(stderr, "Bad input -- child with two mothers on line %d\n", line);
				exit(1);
			}
		}
		else {
			child->mother = parent;
		}
	}
	// Add child to the parent's dllist of children if not already present
	Dllist ptr;
	for (ptr = parent->children->flink; ptr != parent->children; ptr = ptr->flink) {
		Person *curr = (Person *) ptr->val.v;
		if(curr == child) {
			return;
		}
	}
	dll_append(parent->children, new_jval_v(child));
}
// Function to check for and add valid sex to person
void sex_check(Person *p, char *temp_sex, int line) {
	if(p->sex != NULL) {
		if(strcmp(p->sex, temp_sex) !=0) {
			fprintf(stderr, "Bad input - sex mismatch on line %d\n", line);
			exit(1);					
		}
	}
	else {
		p->sex = strdup(temp_sex);
	}
}
/* Function using DFS to check for cycles in the tree. Visiting means currently in the line that 
 * DFS is going through, visited means the node has been exhaustively explored and can be skipped*/
void cycle_check(Person* curr_person) {
	if(curr_person->visiting == 1) {
		fprintf(stderr, "Bad input -- cycle in specification\n");
		exit(1);
	}
	if(curr_person->visited == 1) {
		return;
	}
	curr_person->visiting = 1;
	// Check each childs line to see if it ever points back to a person currently in the line
	Dllist ptr;
    for (ptr = curr_person->children->flink; ptr != curr_person->children; ptr = ptr->flink) {
        Person *p = (Person *) ptr->val.v;
		cycle_check(p);
	}
	curr_person->visited = 1;
	curr_person->visiting = 0;
}
// Function to print the tree topoligically sorted
void print(Person *p) {
	if(p->printed) {
		return;
	}
	// Check if mother and father have been printed first, call print on them if not
	if(p->father != NULL && !p->father->printed) {
		print(p->father);
	}
	if(p->mother != NULL && !p->mother->printed) {
		print(p->mother);
	}
	
    printf("%s\n", p->name);

    if(p->sex != NULL) {printf("  Sex: %s\n", p->sex);}
    else {printf("  Sex: Unknown\n");}

    if(p->father != NULL) {printf("  Father: %s\n", p->father->name);}
    else {printf("  Father: Unknown\n");}

    if(p->mother != NULL) {printf("  Mother: %s\n", p->mother->name);}
    else{printf("  Mother: Unknown\n");}
    if(dll_empty(p->children)) {printf("  Children: None\n");}
    else {
        printf("  Children: \n");
        Dllist ptr;
        for (ptr = p->children->flink; ptr != p->children; ptr = ptr->flink) {
           Person *c = (Person *) ptr->val.v;
           printf("    %s\n", c->name);
        }
    }
    printf("\n");
	p->printed = 1;
}
// Main Function
int main(int argc, char **argv) {
	
	IS is;
	JRB tree;
	
	is = new_inputstruct(argv[1]);

	if (is == NULL) {
		perror(argv[1]);
		exit(1);
	}

	tree = make_jrb();

	Person *curr_person = NULL;
	Person *relative = NULL;
	// Gets each line and determines what attribute its specifying
	while(get_line(is) >= 0) {
		if(is->NF == 0) { // Skip blank lines, AI helped me realize my need for this
			continue;
		}
		if (strcmp(is->fields[0], "PERSON")==0) {
				curr_person = name_check(tree, is);
		}
		/* For these four relationships, they already have pointer to the current person. They name
		   check this relative, check for connections based on father or mother specification, and
		   each one checks the sex to make sure everything is valid. For connection check, you can
		   simply reverse the order of arguments to represent both sides of the connection */
		else if (strcmp(is->fields[0], "FATHER")==0) {
			relative = name_check(tree, is);
			connection_check(curr_person, relative, "M", is->line);
			sex_check(relative, "Male", is->line);
		}
		else if (strcmp(is->fields[0], "MOTHER")==0) {
			relative = name_check(tree, is);
			connection_check(curr_person, relative, "F", is->line);
			sex_check(relative, "Female", is->line);
		}
		else if (strcmp(is->fields[0], "FATHER_OF")==0) {
			relative = name_check(tree, is);
			connection_check(relative, curr_person, "M", is->line);
			sex_check(curr_person, "Male", is->line);
		}
		else if (strcmp(is->fields[0], "MOTHER_OF")==0) {
			relative = name_check(tree, is);
			connection_check(relative, curr_person, "F", is->line);
			sex_check(curr_person, "Female", is->line);
		}
		else if (strcmp(is->fields[0], "SEX")==0) {
			if(strcmp(is->fields[1], "M")==0) {
				sex_check(curr_person, "Male", is->line);
			}
			else {
				sex_check(curr_person, "Female", is->line);
			}
		}
	}
	// Check for cycles running DFS from every node	
	JRB node;
    jrb_traverse(node, tree) {
        Person *p = (Person *) node->val.v;
		cycle_check(p);
	}
	// Go through entire tree to ensure everything gets printed in order
	jrb_traverse(node, tree) {
		Person *p = (Person *) node->val.v;
		print(p);
	}
	
	// Freeing all allocated memory
	jettison_inputstruct(is);
	jrb_traverse(node, tree) {
		Person *p = (Person *) node->val.v;
		free(p->name);
		free(p->sex);
		free_dllist(p->children);
		free(p);
	}
	jrb_free_tree(tree);

	return 0;
}
