/* COSC360 Lab4 Fakemake
   Harrison Reaves
    
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
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
	// filename check
	/*
	printf("filename: %s\n", is->name);
	*/

	char *exe_name = NULL;
	Dllist lists[5]; // 0 = C, 1 = H, 2 = F, 3 = L, 4 = C but with .o instead of .c
	for (int i = 0; i < 5; i++) {lists[i] = new_dllist();}
	
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
			exe_name = strdup(is->fields[1]);   
		}
	}
	
	if(exe_name == NULL) {
        fprintf(stderr, "no executable name\n");
        exit(1);
    }

	Dllist ptr;
	struct stat buf;
	int exists;
	time_t max_h = 0;
	
	dll_traverse(ptr, lists[1]) {
		exists = stat(ptr->val.s, &buf);
		if (exists < 0) {
			fprintf(stderr, "%s not found\n", ptr->val.s);
			exit(1);
		} 
		else {
			if(buf.st_mtime > max_h) {
				max_h = buf.st_mtime;
			}
		}
	}
	size_t len;
	time_t c_time, o_max = 0;
	char *c_file, *o_file;
	char command[1000];

	dll_traverse(ptr, lists[0]) {
		c_file = ptr->val.s;
		exists = stat(c_file, &buf);
        if (exists < 0) {
            fprintf(stderr, "%s not found\n", c_file);
			exit(1);
		}
        else {
			len = strlen(c_file);
            o_file = strdup(c_file);
            o_file[len - 1] = 'o';
            dll_append(lists[4], new_jval_s(o_file));

			c_time = buf.st_mtime;

			exists = stat(o_file, &buf);
			if (exists < 0 || buf.st_mtime < max_h || buf.st_mtime < c_time) {
				sprintf(command, "gcc -c %s", c_file);
				system(command);
			}	
			stat(o_file, &buf);
			if (buf.st_mtime > o_max) {
				o_max = buf.st_mtime;
			}
        }
	}

	exists = stat(exe_name, &buf);
    if (exists < 0 || buf.st_mtime < o_max) {
		command[0] = '\0';
		sprintf(command, "gcc -o %s ", exe_name);
		dll_traverse(ptr, lists[2]) {   // flags
			strcat(command, ptr->val.s);
			strcat(command, " ");
		}
		dll_traverse(ptr, lists[4]) { // .o file
            strcat(command, ptr->val.s);
            strcat(command, " ");
        }
		dll_traverse(ptr, lists[3]) {   // libraries
			strcat(command, ptr->val.s);
			strcat(command, " ");
		}
		system(command);
    }

	// checking dllist values
	/*Dllist ptr;
	for (int i = 0; i < 4; i++) {
		printf("%d: ", i);
		dll_traverse(ptr, lists[i]) printf("%s ", ptr->val.s);
		printf("\n");
	}
	printf("%s\n", name);
	*/

	for (int i = 0; i < 5; i++) {free_dllist(lists[i]);}
	jettison_inputstruct(is);
	return 0;
}
