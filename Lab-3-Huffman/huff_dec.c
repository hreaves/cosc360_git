// COSC360 Lab3 Huffman Decoder
// Harrison Reaves
//
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct huff_node {
    struct huff_node *zero;
    struct huff_node *one;
    char *s_zero;
    char *s_one;
} Node;

Node *new_node() {
	Node *n;
    n = (Node *) malloc(sizeof(Node));
    n->zero = NULL;
    n->one = NULL;
    n->s_zero = NULL;
    n->s_one = NULL;
	return n;
}

void insert_code(Node *root, char *string, char *bits) {
	Node *curr = root;
	for (int i = 0; bits[i] != '\0'; i++) {
		if(bits[i] == 48) {
			if (bits[i+1] != '\0') {
				if(curr->zero == NULL) {
					curr->zero = new_node();
				}
				curr = curr->zero;
			}
			else {
				curr->s_zero = strdup(string);
			}
		}

		else {
			if (bits[i+1] != '\0') {
				if(curr->one == NULL) {
				    curr->one = new_node();
				}
				curr = curr->one;
            }
            else {
                curr->s_one = strdup(string);
            }
		}
	}
}

int main(int argc, char **argv) {

	FILE *f_code, *f_input;
    int input_size_bits, input_size_bytes;

	f_code = fopen(argv[1], "rb");
    f_input = fopen(argv[2], "rb");
    if (f_code == NULL || f_input == NULL) {fprintf(stderr, "incorrect format\n"); exit(1);}

    fseek(f_input, 0, SEEK_END);
	input_size_bytes = ftell(f_input);

	if(input_size_bytes < 4 ) {fprintf(stderr, "Error: file is not the correct size.\n"); exit(1);}

    Node *root = new_node();
	
	int ascii_int = 0, i = 0;
	char ascii_string[10001];
	char ascii_bits[10001];

	while(1) {
		if(ascii_int == EOF) {
			break;
		}
		i = 0;
		while ((ascii_int = fgetc(f_code)) != EOF && ascii_int != '\0') {
			ascii_string[i] = (char) ascii_int;
			i++;
		}
		ascii_string[i] = '\0';
		i = 0;
		while ((ascii_int = fgetc(f_code)) != EOF && ascii_int != '\0') {
			ascii_bits[i] = (char)ascii_int;
			i++;
		}
		ascii_bits[i] = '\0';
		insert_code(root, ascii_string, ascii_bits);
	}


	fseek(f_input, -4, SEEK_END);
    fread(&input_size_bits, sizeof(int), 1, f_input);
	if(input_size_bytes - 4 != (input_size_bits + 7) / 8) {
        fprintf(stderr, "Error: Total bits = %d, but file's size is %d\n", input_size_bits, input_size_bytes); exit(1);
    }
	fseek(f_input, 0, SEEK_SET);
	
	int byte, bit, bit_count = 0;
	Node *curr = root;
	for(int i = 0; i < input_size_bytes; i++) {
		byte = fgetc(f_input);
		for(int j = 0; j < 8; j++) {
			if(bit_count == input_size_bits) {break;}
			bit = (byte >> j) & 1;
			bit_count++;
			if(bit == 1) {
				if(curr->one != NULL) {
					curr = curr->one;
				}
				else if(curr->s_one != NULL) {
					printf("%s", curr->s_one);
					curr = root;
				}
				else {fprintf(stderr, "Unrecognized bits\n"); exit(1);}
			}
			else {
                if(curr->zero != NULL) {
                    curr = curr->zero;
                }
                else if(curr->s_zero != NULL) {
					printf("%s", curr->s_zero);
					curr = root;
                }
				else {fprintf(stderr, "Unrecognized bits\n"); exit(1);}
			}
		}
	}

	fclose(f_code);
    fclose(f_input);

	return 0;   
}
