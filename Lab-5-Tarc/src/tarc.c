/*  COSC360 Lab5 Tarc
	Harrison Reaves
	This program mimics the functionality of tar cf (create). It takes a pathname and creates a tarfile
	with enough information to recreate the directory and all of its contents, printed to standard output.
	Note: I used the Prsize8 program from Plank's notes to help recursively traverse all the directories
	in the given directory. I also used AI to help debug.
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

// Struct for the current directory so both paths can be saved in dllist
typedef struct {
	char *full;
	char *stripped;
} Paths;

// Adding a comparison function for inodes. Plank's notes.
int compare(Jval v1, Jval v2) {
	if (v1.l < v2.l) return -1;
	if (v1.l > v2.l) return 1;
	return 0;
}

// Function to print directories and files with inode existance check
void print(JRB inodes, struct stat buf, char *full_path, char *stripped_path) {
	
	// print filename size, stripped path, inode everytime
	int fn_size = strlen(stripped_path);
	fwrite(&fn_size, 1, 4, stdout);
	fwrite(stripped_path, 1, fn_size, stdout);
	fwrite(&buf.st_ino, 1, 8, stdout);
	
	// check if inode is seen yet, if not, add to tree and print mode and modification time
	if (jrb_find_gen(inodes, new_jval_l(buf.st_ino), compare) == NULL) { 
		jrb_insert_gen(inodes, new_jval_l(buf.st_ino), new_jval_i(0), compare); 
		fwrite(&buf.st_mode, 1, 4, stdout);
		fwrite(&buf.st_mtime, 1, 8, stdout);
		
		// if its a file and not a directory, print file size and bytes in the file
		if (S_ISREG(buf.st_mode)) {
			fwrite(&buf.st_size, 1, 8, stdout);
			
			FILE *file;
			char block[4096];
			size_t bytes_read;

			file = fopen(full_path, "rb");
			if (file == NULL) {perror(full_path); exit(1);}
			
			// read and write the bytes using large buffer
			while((bytes_read = fread(block, 1, sizeof(block), file)) > 0) {
				fwrite(block, 1, bytes_read, stdout);
			}

			fclose(file);
		}
    }
    return;
}

/*  Recursive function to go through all files and subdirectories. Some of it is from Plank's notes.
	I always adjust the full and stripped pathnames in parallel.*/
void traverse(char *fn, char *s_fn, JRB inodes) {
	
	DIR *d; // current open directory
    struct dirent *de; // next directory in loop
    struct stat buf;
    int exists;
	
	int fn_size, s_fn_size; // size of additional path name given as arg
	int dir_fn_size, s_dir_fn_size; // size of full path name
	int sz; // size of current file in the loop
	char *dir_fn, *s_dir_fn; // complete pathname, s = stripped

	Dllist directories, tmp; // For doing recusion after closing directory

	fn_size = strlen(fn);
	s_fn_size = strlen(s_fn);

	dir_fn_size = fn_size + 10;
	s_dir_fn_size = s_fn_size + 10;

	dir_fn = (char *) malloc(sizeof(char) * dir_fn_size);
	s_dir_fn = (char *) malloc(sizeof(char) * s_dir_fn_size);

	if (dir_fn == NULL) { perror("malloc dir_fn"); exit(1); } 
	if (s_dir_fn == NULL) { perror("malloc s_dir_fn"); exit(1); }
	
	// new full path starts with given path
	strcpy(dir_fn, fn);
	strcpy(s_dir_fn, s_fn);

	// add / to end of new full path name
	strcat(dir_fn + fn_size, "/");
	strcat(s_dir_fn + s_fn_size, "/");

	// open directory at this path and error if nonexistent
	d = opendir(fn);
    if (d == NULL) {
        perror(fn);
        exit(1);
    }
	directories = new_dllist();

	// go through all files in the current directory
	for (de = readdir(d); de != NULL; de = readdir(d)) {
		
		// skip over . and ..
		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {continue;}
		
		sz = strlen(de->d_name);
		
		// check if enough room to add file name ontop of path name, if not, add enough
		if (dir_fn_size < fn_size + sz + 2) {
			dir_fn_size = fn_size + sz + 10;
			dir_fn = realloc(dir_fn, dir_fn_size);
		}
		if (s_dir_fn_size < s_fn_size + sz + 2) { 
            s_dir_fn_size = s_fn_size + sz + 10;   
            s_dir_fn = realloc(s_dir_fn, s_dir_fn_size);
        }
		
		// add file name to the end of path name after /
		strcpy(dir_fn + fn_size + 1, de->d_name); 
		strcpy(s_dir_fn + s_fn_size + 1, de->d_name);

		// check if file actually exists, skip if not
		exists = lstat(dir_fn, &buf);
		if (exists < 0) {
			fprintf(stderr, "Couldn't stat %s\n", dir_fn);
			exit(1);
		}	 
		
		// skip symbolic links
		if (S_ISLNK(buf.st_mode)) {continue;}

		// check if directory, add to dllist if it is
		if (S_ISDIR(buf.st_mode)) { 
			Paths *p = malloc(sizeof(Paths));
			p->full = strdup(dir_fn);
			p->stripped = strdup(s_dir_fn);
			dll_append(directories, new_jval_v(p));
		}
		// print the current directory or file
		print(inodes, buf, dir_fn, s_dir_fn); 
	}

	// close current directory and traverse through the subdirectories in the dllist
	closedir(d);
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

// Main
int main(int argc, char **argv) {

	if(argc != 2) {
		fprintf(stderr, "Improper input\n");
        exit(1);
	}
	
	JRB inodes; // holds which inodes we have already seen
	inodes = make_jrb();

	char *root, *strip_root;
	root = strdup(argv[1]);
	strip_root = strdup(basename(root)); // AI showed me basename to help strip the root
	
	struct stat buf;
	int exists;
	
	// open given root direcetory, given as arg, and print details then start traversal
	exists = lstat(root, &buf);
	if (exists < 0) {
        fprintf(stderr, "Couldn't stat %s\n", root);
        exit(1);
    }
	print(inodes, buf, root, strip_root);
	traverse(root, strip_root, inodes);
	
	// clean up
	free(root);
	free(strip_root);
	jrb_free_tree(inodes);

	return 0;
}

