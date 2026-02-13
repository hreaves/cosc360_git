// COSC360 Lab3 Huffman Decoder
// Harrison Reaves
//
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct huff_node {
	struct huff_node *zero;
	struct huff_node *one;
	char *s_zero;
	char *s_one;
}; Node

int path(Node *root, int ascii, File *f_code, int j, int final_val) {

	int val = fgetc(f_code);
    if((val != 30 || val != 31) && j == 0) {
		perror("code file improper format");
        exit(0);
    }
	if (val != 30 || val != 31) {
        if(final_val == 30) {
			
		}
		else {
			
		}

		return j;
    }
    j++;
	final_val = val;
	if(root->zero == NULL && val == 30) {
		Node *new_node = malloc(sizeof(Node));
		new_node->zero = NULL;
		new_node->one = NULL;
		new_node->s_zero = NULL;
		new_node->s_one = NULL;
		root->zero = new_node;
		path(new_node, ascii, f_code, j+1, final_val);
	}
	else {
		path(root->zero, ascii, f_code, j+1, final_val);
	}

	if(root->one == NULL && val == 31) {
		Node *new_node = malloc(sizeof(Node));
        new_node->zero = NULL;
        new_node->one = NULL;
        new_node->s_zero = NULL;
        new_node->s_one = NULL;
        root->one = new_node;
		path(new_node, ascii, f_code, j+1, final_val);
	}
	else {
		path(root->first, ascii, f_code, j+1, final_val);
	}
}


int main(int argc, char **argv) {
	
	File *f_code, *f_input;
	int f_code_size1, f_input_size;
	int *code, *input;
	int ascii, skip;

	f_code = open(argv[1], O_RDONLY);
	f_input = open(argv[2], O_RDONLY);
	if (f_code < 0 || f_input < 0) { 
		perror("incorrect format"); exit(1); 
	}

	// get last four bytes and file size, then return to beginning
	fseek(f_code, -4, SEEK_END);
	fread(f_code_size1, sizeof(int), 1, f_code);
	fseek(f_code, 0, SEEK_SET);
	
	Node *root;
	root = (Node *) malloc(sizeof(Node));
	root->zero = NULL;
    root->one = NULL;
    root->s_zero = NULL;
    root->s_one = NULL;

	while (1) {
		ascii = fgetc(f_code);
		if((ascii == EOF) && (i != (f_code_size/8) + 4 )) {
			perror("code file too small");
			exit(0);
		}
		if(ascii == EOF) {
            return;
        }
		skip = path(root, ascii, f_code, 0);
		i += skip + 1;
		fseek(f_code, skip, SEEK_CUR);
	}



	// After reading code
	if(f_code_size1 != f_code_size2 - 4) {
		perror("mismatched code sizes"); exit(1);	
	}
	// after reading input
	if(f_input_size < 4) {
		perror("input file too small"); exit(1);
	}

	fclose(f_code);
	fclose(f_input);

	return 0;
}
