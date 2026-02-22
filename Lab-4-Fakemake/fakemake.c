/* COSC360 Lab4 Fakemake
   Harrison Reaves
   This program helps you automate compiling, similar to make, but is limited to one executable
   and assumes that you are using gcc to do your compilation.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "fields.h"
#include "dllist.h"
#include "jval.h"
#include <sys/wait.h>
// function to free all memory before exiting
void free_mem(Dllist *lists, IS is, char *exe_name) {
	Dllist ptr;
	for (int i = 0; i < 5; i++) {
		dll_traverse(ptr, lists[i]) {
			free(ptr->val.s);
		}
		free_dllist(lists[i]);
	}
    jettison_inputstruct(is);
    free(exe_name);
}
// main function
int main(int argc, char **argv) {
  
	IS is;
	if(argc == 1) {is = new_inputstruct("fmakefile");}
	else {is = new_inputstruct(argv[1]);}
	char *exe_name = NULL;
	Dllist lists[5]; // 0 = C, 1 = H, 2 = F, 3 = L, 4 = C but with .o instead of .c
    for (int i = 0; i < 5; i++) {lists[i] = new_dllist();} // allocate memory for all dllists

	if (is == NULL) {
		printf("Input file does not exist\n");
		free_mem(lists, is, exe_name);
		exit(1);
	}
	
	int e_cnt = 0; // count to ensure E has no more than one line
	// cycle through every line, add argument to corresponding dllist in the array
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
			if(e_cnt> 0) {
				fprintf(stderr, "fmakefile (%d) cannot have more than one E line\n", is->line);
				free_mem(lists, is, exe_name);
				exit(1);
			}
			exe_name = strdup(is->fields[1]);
			e_cnt++;
		}
	}
	
	if(exe_name == NULL) {
        fprintf(stderr, "No executable specified\n");
        free_mem(lists, is, exe_name);
		exit(1);
    }

	Dllist ptr;
	struct stat buf;
	int exists;
	time_t max_h = 0; // lastest helper file change
	// traverse through the .h file dllist, ensure every file exists, then do comparison to maintain lastest time change
	dll_traverse(ptr, lists[1]) {
		exists = stat(ptr->val.s, &buf);
		if (exists < 0) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", ptr->val.s);
			free_mem(lists, is, exe_name);
			exit(1);
		} 
		else {
			if(buf.st_mtime > max_h) {
				max_h = buf.st_mtime;
			}
		}
	}
	size_t len; // for size of c file
	time_t c_time, o_max = 0; // time of change for given .c file and latest change for all .o files
	char *c_file, *o_file; 
	char command[1000]; // for system call
	int status; // to check system call success
	Dllist ptr_2;
	// ensure all .c files exist, check for corresponding .o files, compile if necessary
	dll_traverse(ptr, lists[0]) {
		c_file = ptr->val.s;
		exists = stat(c_file, &buf);
        if (exists < 0) {
            fprintf(stderr, "fmakefile: %s: No such file or directory\n", c_file);
			free_mem(lists, is, exe_name);
			exit(1);
		}
        else { 
			// switch filenames from .c to .o and store in another dllist, AI helped me with these next 3 lines
			len = strlen(c_file);
            o_file = strdup(c_file);
            o_file[len - 1] = 'o';
            dll_append(lists[4], new_jval_s(o_file));

			c_time = buf.st_mtime;
			// if .o file doesnt exist or a helper file or the c file has been changed more recently, recompile
			exists = stat(o_file, &buf);
			if (exists < 0 || buf.st_mtime < max_h || buf.st_mtime < c_time) {
				sprintf(command, "gcc -c ");
				dll_traverse(ptr_2, lists[2]) { // add all flags
					strcat(command, ptr_2->val.s);
					strcat(command, " ");
				}
				strcat(command, c_file); 
				printf("%s\n", command);
				status = system(command);
				// check to ensure no compilation issues. AI helped me with the WEXITSTATUS flag since I didn't know how to do it
				if (status == -1 || WEXITSTATUS(status) != 0) {
					fprintf(stderr, "Command failed.  Exiting\n");
					free_mem(lists, is, exe_name);
					exit(1);
				}
			}
			// rerun stat to get up to date .o file modification time and ensure latest time is kept
			stat(o_file, &buf);
			if (buf.st_mtime > o_max) {
				o_max = buf.st_mtime;
			}
        }
	}
	// if the executable doesn't exist or a .o file has been modified more recently than it, compile
	exists = stat(exe_name, &buf);
    if (exists < 0 || buf.st_mtime < o_max) {
		command[0] = '\0'; // clear the buffer
		sprintf(command, "gcc -o %s", exe_name);
		dll_traverse(ptr, lists[2]) {   // flags
			strcat(command, " ");
			strcat(command, ptr->val.s);
		}
		dll_traverse(ptr, lists[4]) { // .o files
			strcat(command, " ");
            strcat(command, ptr->val.s);
        }
		dll_traverse(ptr, lists[3]) {   // libraries
			strcat(command, " ");
			strcat(command, ptr->val.s);
		}

		printf("%s\n", command);
		// same system error checking as earlier
		status = system(command);
		if (status == -1 || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command failed.  Fakemake exiting\n");
            free_mem(lists, is, exe_name);
			exit(1);
		}
    } // if exists and doesnt need to be recompiled, leave it be
	else {
		printf("%s up to date\n", exe_name);
	}

	free_mem(lists, is, exe_name);

	return 0;
}
