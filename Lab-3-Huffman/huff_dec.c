/* COSC360 Lab3 Huffman Decoder
   Harrison Reaves
   This program takes a code definition file and an encrypted input file, and it uses the code
   definition file to decode the input file and output to standard output.The code associates
   strings with sequences of bits.
   AI was used to aid in debugging.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct huff_node {
    struct huff_node *zero;
    struct huff_node *one;
    char *s_zero;
    char *s_one;
} Node;
// Function creates a new node for the tree and initializes fields to NULL
Node *new_node() {
	Node *n;
    n = (Node *) malloc(sizeof(Node));
    n->zero = NULL;
    n->one = NULL;
    n->s_zero = NULL;
    n->s_one = NULL;
	return n;
}
// Function recursively frees memory for entire tree
void free_tree(Node *n) {
	if(n==NULL) {return;}
	free_tree(n->zero);
	free_tree(n->one);
	free(n->s_zero);
	free(n->s_one);
	free(n);
	return;
}
// Function to insert new nodes into the tree
void insert_code(Node *root, char *string, char *bits) {
	Node *curr = root;
	// for every bit, check if there's an existent path. If not, create one and move to it.
	for (int i = 0; bits[i] != '\0'; i++) {
		if(bits[i] == 48) { // 48 == 0
			if (bits[i+1] != '\0') {
				if(curr->zero == NULL) {
					curr->zero = new_node();
				}
				curr = curr->zero;
			}
			// if this is the last bit in the sequence, store the string literal in the curr node
			else {
				curr->s_zero = strdup(string);
			}
		}
		// same thing but for when bit is 0 (49)
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
// Main function
int main(int argc, char **argv) {

	FILE *f_code, *f_input;
    int input_size_bits, input_size_bytes;

	// open files binary read only and ensure they're not empty
	f_code = fopen(argv[1], "rb");
    f_input = fopen(argv[2], "rb");
    if (f_code == NULL || f_input == NULL) {fprintf(stderr, "incorrect format\n"); exit(1);}
	// get actual size of input file in bytes
    fseek(f_input, 0, SEEK_END);
	input_size_bytes = ftell(f_input);

	if(input_size_bytes < 4 ) {fprintf(stderr, "Error: file is not the correct size.\n"); exit(1);}

    Node *root = new_node();
	
	int ascii_int = 0, i = 0;
	char ascii_string[10001];
	char ascii_bits[10001];
	// loop through each pair of sequences, stopping when reaching end of file (EOF)
	while(1) {
		if(ascii_int == EOF) {
			break;
		}
		i = 0;
		// loop through each char in string sequence, store in array char
		while ((ascii_int = fgetc(f_code)) != EOF && ascii_int != '\0') {
			ascii_string[i] = (char) ascii_int;
			i++;
		}
		ascii_string[i] = '\0';
		i = 0;
		// loop through each char in the bit sequence. store in array as char
		while ((ascii_int = fgetc(f_code)) != EOF && ascii_int != '\0') {
			ascii_bits[i] = (char)ascii_int;
			i++;
		}
		ascii_bits[i] = '\0';
		// create new path with these two sequences
		insert_code(root, ascii_string, ascii_bits);
	}

	// get the stated file size from the last 4 bytes of the file, ensure that it matches the actual size
	fseek(f_input, -4, SEEK_END);
    fread(&input_size_bits, sizeof(int), 1, f_input);
	if(input_size_bytes - 4 != (input_size_bits + 7) / 8) {
        fprintf(stderr, "Error: Total bits = %d, but file's size is %d\n", input_size_bits, input_size_bytes); exit(1);
    }
	fseek(f_input, 0, SEEK_SET);

	int byte, bit, bit_count = 0;
	Node *curr = root;
	// loop through every byte in the input file until reach total byte size reached
	for(int i = 0; i < input_size_bytes; i++) {
		byte = fgetc(f_input);
		// loop through each bit in the given byte
		for(int j = 0; j < 8; j++) {
			
			if(bit_count == input_size_bits) {break;}
			// mask to get each bit alone starting with LSB
			bit = (byte >> j) & 1;
			bit_count++;
			// go to next node in tree if it exists, if not, output current node's string
			if(bit == 1) {
				if(curr->one != NULL) {
					curr = curr->one;
				}
				else if(curr->s_one != NULL) {
					printf("%s", curr->s_one);
					curr = root; // restart curr to move onto next sequence of bits since the current sequence is finished
				}
				// no next node and no current string
				else {fprintf(stderr, "Unrecognized bits\n"); exit(1);}
			}
			// same thing but for 0
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

	free_tree(root);

	return 0;   
}
