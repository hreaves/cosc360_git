/* COSC360 Lab4 Fakemake
   Harrison Reaves
    
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fields.h"
#include "dllist.h"
#include "jval.h"

int main(int argc, char **argv) {
  
	IS is;	

	if(argc == 1) {is = new_inputstruct("fmakefile");}
	else {is = new_inputstruct(argv[1]);}
	
	if (is == NULL) {
		printf("Input file does not exist\n");
		exit(1);
	}
	printf("filename: %s\n", is->name);

	char *name = NULL;
	Dllist lists[4]; // 0 = C, 1 = H, 2 = F, 3 = L
	for (int i = 0; i < 4; i++) {lists[i] = new_dllist();}
	
	while(get_line(is) >= 0) {
		if(is->NF == 0) {continue;}  // skip blank lines
		if(strcmp(is->fields[0], "C")==0) {
			for (int i = 1; i < is->NF; i++) {
				dll_append(lists[0], new_jval_s(strdup(is->fields[i])));
			}
		}
		else if(strcmp(is->fields[0], "H")==0) {
            for (int i = 1; i < is->NF; i++) {
                dll_append(lists[1], new_jval_s(strdup(is->fields[i])));
            }
		}
		else if(strcmp(is->fields[0], "F")==0) {
            for (int i = 1; i < is->NF; i++) {
                dll_append(lists[2], new_jval_s(strdup(is->fields[i])));
            }
		}
		else if(strcmp(is->fields[0], "L")==0) {
            for (int i = 1; i < is->NF; i++) {
                dll_append(lists[3], new_jval_s(strdup(is->fields[i])));
            }
		}
		else if(strcmp(is->fields[0], "E")==0) {
			name = strdup(is->fields[1]);   
		}
		for (int i = 0; i < is->NF; i++) {
			printf("%s ", is->fields[i]);
		}
		printf("\n");
		if(name == NULL) {
			printf("no executable name\n");
			exit(1);
		}
	}
	Dllist ptr;
	for (int i = 0; i < 4; i++) {
		printf("%d: ", i);
		dll_traverse(ptr, lists[i]) printf("%s ", ptr->val.s);
		printf("\n");
	}
	printf("%s\n", name);

	for (int i = 0; i < 4; i++) {free_dllist(lists[i]);}
	jettison_inputstruct(is);
	return 0;
}
