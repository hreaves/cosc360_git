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
	int adj_size;
	struct node **adj;
	int visited;
} Node;

typedef struct global {
	int num_jumps; 
	int	best_heal;
    double power_reduction;
	int best_path_length;
	Node ** best_path;
	int *healing;

} Global;

void DFS(Node *cur_n, int hop_num, Global *g, int total_heal, double cur_power, Node *from) {
	
	if(cur_n->visited == 1 || hop_num > g->num_jumps) {
		return;
	}
	cur_n->visited = 1;
	
	if(cur_power + cur_n->cur_PP > cur_n->max_PP) {
        total_heal += cur_n->max_PP - cur_n->cur_PP;
	}
    else {
	    total_heal += cur_power;
	}
	if(total_heal > g->best_heal) {
		g->best_heal = total_heal;
	}
	cur_power = rint(cur_power * (1 - g->power_reduction));

	for(int i = 0; i < cur_n->adj_size; i++) {
		DFS(cur_n->adj[i], hop_num + 1, g, total_heal, cur_power, cur_n);
	}
	cur_n->visited = 0;
}


int main(int argc, char **argv) {
	int initial_range, jump_range, initial_power;

	Global *g;
	g = (Global *) malloc(sizeof(Global));
	initial_range = atoi(argv[1]);
	jump_range = atoi(argv[2]);
	g->num_jumps = atoi(argv[3]);
	initial_power = atoi(argv[4]);
	g->power_reduction = atof(argv[5]);
	g->best_heal = 0;
	g->best_path_length = 0;
	g->best_path = (Node **) malloc(g->num_jumps * sizeof(Node *));
	g->healing = (int *) malloc(g->num_jumps * sizeof(int));

	char name[100];
	int x, y, cur_PP, max_PP, num_nodes = 0;
	Node *head = NULL, *prev = NULL;
	while(scanf("%d %d %d %d %99s", &x, &y, &cur_PP, &max_PP, name) == 5) { 
		Node *n;
		n = (Node *) malloc(sizeof(Node));
		n->x = x;
		n->y = y;
		n->cur_PP = cur_PP;
		n->max_PP = max_PP;
		n->name = (char *) malloc(100 * sizeof(char));
		strcpy(n->name, name);
		num_nodes++;
		n->prev = prev;
		n->visited = 0;
		n->adj_size = 0;
		prev = n;
		head = n;
	}

	Node **nodes;
	nodes = (Node **) malloc(num_nodes * (sizeof(Node*)));
	for(int i = 0; i < num_nodes; i++) {
		nodes[i] = head;
		head = head->prev;
	}
	// calculate number of adjacenies
	int x_dist, y_dist;
	for(int i = 0; i < num_nodes; i++) {
		for(int j = 0; j < num_nodes; j++) {
			if(i==j) {
				continue;
			}
			x_dist = nodes[i]->x - nodes[j]->x;
			y_dist = nodes[i]->y - nodes[j]->y;
			if((x_dist*x_dist + y_dist*y_dist) <= (jump_range*jump_range)) {
				nodes[i]->adj_size++;
			}
		}
		nodes[i]->adj = (Node **) malloc(nodes[i]->adj_size * sizeof(Node*));
	}
	// fill up the adjacency lists
	int k;
	for(int i = 0; i < num_nodes; i++) {
		k = 0;
        for(int j = 0; j < num_nodes; j++) {
			if(i==j) {
				continue;
			}
            x_dist = nodes[i]->x - nodes[j]->x;
            y_dist = nodes[i]->y - nodes[j]->y;
			if((x_dist*x_dist + y_dist*y_dist) <= (jump_range*jump_range)) {
                nodes[i]->adj[k] = nodes[j];
				k++;
			}
		}
	}
	// find nodes within initial range of Urgosa and run DFS on them
	for(int i = 0; i < num_nodes; i++) {
		if(strcmp(nodes[i]->name, "Urgosa_the_Healing_Shaman") == 0) {
			for(int j = 0; j < num_nodes; j++) {
				x_dist = nodes[i]->x - nodes[j]->x;
				y_dist = nodes[i]->y - nodes[j]->y;
				if((x_dist*x_dist + y_dist*y_dist) <= (initial_range*initial_range)) {
					DFS(nodes[j], 1, g, 0, initial_power, NULL); // starting node, hop_num, pointer to globals, total healing, starting power
				}
			}
		}		
	}
	printf("Total Healing: %d \n", g->best_heal);
 

return 0; }
