/*  COSC360 Lab6 TARX
	Harrison Reaves

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

	char command[1000];
	char block[4096];

	JRB inodes;
	inodes = make_jrb();
	Dllist times, tmp;
	times = new_dllist();

	struct timeval t[2];

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
		if (jrb_find_gen(inodes, new_jval_l(f_inode), compare) == NULL) {
			Data *data = malloc(sizeof(Data));
            data->name = strdup(fn);
			jrb_insert_gen(inodes, new_jval_l(f_inode), new_jval_v(data), compare); 
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

			dll_append(times, new_jval_v(data));

			if (S_ISREG(data->mode)) {
				if (fread(&f_size, 1, 8, stdin) != 8) {
					fprintf(stderr, "Bad tarfile\n");
					clean(times, inodes, fn);
					exit(1);
				}
				FILE *fp = fopen(fn, "wb");
				byte_count = 0;
				while(byte_count < f_size) {
					read_bytes = 4096;
					if(f_size - byte_count < 4096) {
						read_bytes = f_size - byte_count;
					}	
					if (fread(block, 1, read_bytes, stdin) != read_bytes) {
						fprintf(stderr, "Bad tarfile\n");
						clean(times, inodes, fn);
						fclose(fp);
						exit(1);
					}
					fwrite(block, 1, read_bytes, fp);
					byte_count += read_bytes;
				}
				fclose(fp);
			}
			else {
				mkdir(fn, 0700);				
			}
		}
		else {
			JRB node = jrb_find_gen(inodes, new_jval_l(f_inode), compare);
            link(((Data *)node->val.v)->name, fn);
		}
		free(fn);
	}

	dll_rtraverse(tmp, times) {
		Data *data = (Data *) tmp->val.v;
		t[0].tv_sec = time(NULL); // Access time (atime)
		t[0].tv_usec = 0;
		t[1].tv_sec = data->mtime;   // Modification time (mtime)
		t[1].tv_usec = 0;
		utimes(data->name, t);
		chmod(data->name, data->mode);
	}
	
	clean(times, inodes, NULL);

	return 0;
}
