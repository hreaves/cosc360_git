/*  COSC360 Lab6 TARX
	Harrison Reaves
	This program recreates a directory from a tarfile that was created with tarc,
	including all file and directory protections and modification times. It reads
	the tarfile from standard input.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include "dllist.h"
#include "jval.h"
#include "jrb.h"

typedef struct {
	char *name;
    int mode;
	long mtime;
} Data;

// Adding a comparison function for inodes. Plank's notes.
int compare(Jval v1, Jval v2) {
	if (v1.l < v2.l) return -1;
	if (v1.l > v2.l) return 1;
	return 0;
}
// Function to clear all memory before exiting
void clean(Dllist times, JRB inodes, char *fn) {
	Dllist tmp;
	dll_traverse(tmp, times) {
        Data *data = (Data *) tmp->val.v;
        free(data->name);
        free(data);
    }
    free_dllist(times);
    jrb_free_tree(inodes);
	free(fn);
}

int main() {
	
	long f_inode, f_size, byte_count;
	int fn_size, read_bytes, fn_size_check;
	char *fn;

	char block[4096]; // buffer for reading/writing

	JRB inodes; // all seen inodes
	inodes = make_jrb();
	Dllist times, tmp; // holds data struct for every inode to chmod later
	times = new_dllist();

	struct timeval t[2]; 
	// read through all of stdin, check for errors every time using fread
	while ((fn_size_check = fread(&fn_size, 1, 4, stdin)) != 0) {
		if (fn_size_check != 4) {
			fprintf(stderr, "Bad tarfile\n");
			clean(times, inodes, fn);
			exit(1);
		}
		fn = (char *) malloc(fn_size + 1);
		if (fread(fn, 1, fn_size, stdin) != fn_size) {
			fprintf(stderr, "Bad tarfile\n"); 
			clean(times, inodes, fn);
			exit(1);
		}
		fn[fn_size] = '\0';
		if (fread(&f_inode, 1, 8, stdin) != 8) {
			fprintf(stderr, "Bad tarfile\n");
			clean(times, inodes, fn);
			exit(1);
		}
		// check if inode has already been read, if not, create data struct and add to JRB tree
		if (jrb_find_gen(inodes, new_jval_l(f_inode), compare) == NULL) {
			Data *data = malloc(sizeof(Data));
            data->name = strdup(fn);
			jrb_insert_gen(inodes, new_jval_l(f_inode), new_jval_v(data), compare); 
			// since its new inode, you need mtime and mode as well
			if (fread(&data->mode, 1, 4, stdin) != 4) {
				fprintf(stderr, "Bad tarfile\n");
				clean(times, inodes, fn);
				exit(1);
			}
			if (fread(&data->mtime, 1, 8, stdin) != 8) {
				fprintf(stderr, "Bad tarfile\n"); 
				clean(times, inodes, fn);
				exit(1);
			}
			// add struct to dllist for later
			dll_append(times, new_jval_v(data));
			// if its regular file, then we need to read f_size and bytes of file
			if (S_ISREG(data->mode)) {
				if (fread(&f_size, 1, 8, stdin) != 8) {
					fprintf(stderr, "Bad tarfile\n");
					clean(times, inodes, fn);
					exit(1);
				}
				FILE *fp = fopen(fn, "wb");
				byte_count = 0;
				// read all bytes from stdin with blocks of 4096
				while(byte_count < f_size) {
					read_bytes = 4096;
					// last block
					if(f_size - byte_count < 4096) {
						read_bytes = f_size - byte_count;
					}
					if (fread(block, 1, read_bytes, stdin) != read_bytes) {
						fprintf(stderr, "Bad tarfile\n");
						clean(times, inodes, fn);
						fclose(fp);
						exit(1);
					}
					// write to the newly opened file
					fwrite(block, 1, read_bytes, fp);
					byte_count += read_bytes;
				}
				fclose(fp);
			}
			// if not regular file, its a directory, so mkdir the name
			else {
				mkdir(fn, 0700);				
			}
		}
		// if its not a new inode, create a hard link with the current filename and the already existant filename
		else {
			JRB node = jrb_find_gen(inodes, new_jval_l(f_inode), compare);
            link(((Data *)node->val.v)->name, fn);
		}
		free(fn);
	}
	// Editing metadata for each inode and then changing permissions. AI helped me with this.
	dll_rtraverse(tmp, times) {
		Data *data = (Data *) tmp->val.v;
		t[0].tv_sec = time(NULL); // Access time
		t[0].tv_usec = 0;
		t[1].tv_sec = data->mtime; // Modification time
		t[1].tv_usec = 0;
		utimes(data->name, t);
		chmod(data->name, data->mode);
	}
	
	clean(times, inodes, NULL);

	return 0;
}
