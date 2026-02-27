/*  COSC360 Lab5 Tarc
	Harrison Reaves
	
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h> // basename
#include "dllist.h"
#include "jval.h"
#include "jrb.h"

typedef struct {
	char *full;
	char *stripped;
} Paths;

// Adding a comparison function for inodes.
int compare(Jval v1, Jval v2) {
	if (v1.l < v2.l) return -1;
	if (v1.l > v2.l) return 1;
	return 0;
}
// prints directories and files with inode existance check
void print(JRB inodes, struct stat buf, char *full_path, char *stripped_path) {
	// print filename size, stripped path, inode
	int fn_size = strlen(stripped_path);
	fwrite(&fn_size, 1, 4, stdout);
	fwrite(stripped_path, 1, fn_size, stdout);
	fwrite(&buf.st_ino, 1, 8, stdout);
	if (jrb_find_gen(inodes, new_jval_l(buf.st_ino), compare) == NULL) { // check if inode seen yet
        jrb_insert_gen(inodes, new_jval_l(buf.st_ino), new_jval_i(0), compare); // add inode to tree if not
		// print mode, modification time
		fwrite(&buf.st_mode, 1, 4, stdout);
		fwrite(&buf.st_mtime, 1, 8, stdout);
		if (S_ISREG(buf.st_mode)) {
			// print file size
			fwrite(&buf.st_size, 1, 8, stdout);
			FILE *file;
			char block[4096];
			size_t n;
			file = fopen(full_path, "rb");
			if (file == NULL) {perror(full_path); exit(1);}
			// read and print the bytes
			while((n = fread(block, 1, sizeof(block), file)) > 0) {
				fwrite(block, 1, n, stdout);
			}
			fclose(file);
		}
    }
    return;
}
// Recursive functions to go through all files and subdirectories
void traverse(char *fn, char *s_fn, JRB inodes) {
	
	DIR *d; // current open directory
    struct dirent *de; // next directory for loop
    struct stat buf; // stat buffer
    int exists;

	int fn_size, dir_fn_size, sz, s_fn_size, s_dir_fn_size;
	char *dir_fn, *s_dir_fn; // filename with directory

	Dllist directories, tmp; // Dllist of directory names, for doing recusion after closing

	fn_size = strlen(fn); // size of given path name
	s_fn_size = strlen(s_fn);

	dir_fn_size = fn_size + 10; // full path size + 10 for extra space
	s_dir_fn_size = s_fn_size + 10;

	dir_fn = (char *) malloc(sizeof(char) * dir_fn_size); // allocate memory for new full path
	s_dir_fn = (char *) malloc(sizeof(char) * s_dir_fn_size);

	if (dir_fn == NULL) { perror("malloc dir_fn"); exit(1); } 
	if (s_dir_fn == NULL) { perror("malloc s_dir_fn"); exit(1); }

	strcpy(dir_fn, fn); // new full path starts with given path
	strcpy(s_dir_fn, s_fn);

	strcat(dir_fn + fn_size, "/"); // add / to end of new path name
	strcat(s_dir_fn + s_fn_size, "/");

	d = opendir(fn); // open directory at this path and error if nonexistent, for first case
    if (d == NULL) {
        perror(fn);
        exit(1);
    }

	directories = new_dllist();

	// go through all files in the current directory
	for (de = readdir(d); de != NULL; de = readdir(d)) {

		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {continue;} // skip over . and ..
		sz = strlen(de->d_name); // size of current file in directory
		
		if (dir_fn_size < fn_size + sz + 2) { // check if enough room to add file name ontop of path name
			dir_fn_size = fn_size + sz + 10;   // add enough for file name + 10
			dir_fn = realloc(dir_fn, dir_fn_size); // reallocate
		}
		if (s_dir_fn_size < s_fn_size + sz + 2) { // check if enough room to add file name ontop of path name
            s_dir_fn_size = s_fn_size + sz + 10;   // add enough for file name + 10
            s_dir_fn = realloc(s_dir_fn, s_dir_fn_size); // reallocate
        }

		strcpy(dir_fn + fn_size + 1, de->d_name); // add file name to the end of path name after /
		strcpy(s_dir_fn + s_fn_size + 1, de->d_name); // add file name to the end of path name after /

		exists = lstat(dir_fn, &buf); // check if file actually exists, skip if not
		if (exists < 0) {
			fprintf(stderr, "Couldn't stat %s\n", dir_fn);
			exit(1);
		}	 

		if (S_ISLNK(buf.st_mode)) {continue;} // skip symbolic links
		
		if (S_ISDIR(buf.st_mode)) { // check if directory, add to dllist if it is
			Paths *p = malloc(sizeof(Paths));
			p->full = strdup(dir_fn);
			p->stripped = strdup(s_dir_fn);
			dll_append(directories, new_jval_v(p));
		}
		print(inodes, buf, dir_fn, s_dir_fn); 
	}

	closedir(d); // close current directory and go to next ones
	dll_traverse(tmp, directories) {
		Paths *p = (Paths *) tmp->val.v;
		traverse(p->full, p->stripped, inodes);
	}
	// clean up
	dll_traverse(tmp, directories) {
		Paths *p = (Paths *) tmp->val.v;
		free(p->full);
		free(p->stripped);
		free(p);
	}
	free_dllist(directories);
	free(dir_fn);
	free(s_dir_fn);
	
  return;
}

int main(int argc, char **argv) {

	if(argc != 2) {
		fprintf(stderr, "Improper input\n");
        exit(1);
	}
	JRB inodes;
	inodes = make_jrb();
	char *root, *strip_root;
	
	root = strdup(argv[1]);
	strip_root = strdup(basename(root));
	

	struct stat buf;
	int exists;
	exists = lstat(root, &buf);
	if (exists < 0) {
        fprintf(stderr, "Couldn't stat %s\n", root);
        exit(1);
    }
	print(inodes, buf, root, strip_root);
	traverse(root, strip_root, inodes);
	
	free(root);
	free(strip_root);
	jrb_free_tree(inodes);

	return 0;
}
