// COSC360 Lab1 Chain Heal
// Harrison Reaves
// Using DFS to get an optimal healing path
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


typedef struct node {
  char *name;
  int x, y;
  int cur_PP, max_PP;
  struct node *prev;
} Node;


int main(int argc, char **argv) {
	int initial_range, jump_range, num_jumps, initial_power;
	double power_reduction;

	initial_range = atoi(argv[1]);
	jump_range = atoi(argv[2]);
	num_jumps = atoi(argv[3]);
	initial_power = atoi(argv[4]);
	power_reduction = atof(argv[5]);

	char name[100];
	int x, y, cur_PP, max_PP, num_nodes = 0;
	Node *head = NULL, *prev = NULL;
	while(scanf("%d %d %d %d %99s", &x, &y, &cur_PP, &max_PP, name)==5) { 
		Node *n;
		n = (Node *) malloc(sizeof(Node));
		n->x = x;
		n->y = y;
		n->cur_PP = cur_PP;
		n->max_PP = max_PP;
		n->name = name;
		num_nodes++;
		n->prev = prev;
		prev = n;
		head = n;
	}

	Node **nodes;
	nodes = (Node **) malloc(num_nodes * (sizeof(Node*)));
	for(int i = 0; i < num_nodes; i++) {
		nodes[i] = head;
		head = head->prev;
	}


return 0; }
