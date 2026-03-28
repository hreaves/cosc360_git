/*  COSC356 Lab2 Cache Simulator
	Harrison Reaves
	This program takes a trace of addresses and outputs the total memory accesses, hits, misses, and miss
	rate.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// cache block metadata
typedef struct {
	unsigned long tag;
	int date; // the smaller the date, the later it was changed
	int valid; // if the block has a value already
} Block;

int main(int argc, char **argv) {
	
	int sets, blocks, size, prefetch = 0;
	FILE *file;
	char *filename;
	// input parsing
	for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sets") == 0) {
            sets = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--blocks") == 0) {
            blocks = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--size") == 0) {
            size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--trace") == 0) {
            filename = argv[++i];
        }
		else if (strcmp(argv[i], "--prefetch") == 0) {
			prefetch = 1;
		}
        else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }
	// make sure every arg was given proper value
	if (sets <= 0 || blocks <= 0 || size <= 0 || filename == NULL) {
		fprintf(stderr, "Usage: %s --sets <num> --blocks <num> --size <num> --trace <file>\n", argv[0]);
		return 1;
	}
	// allocate memory for an arrray to cover entire cache needed
	Block *cache = malloc(sets*blocks*sizeof(Block));
	if (cache == NULL) {
		perror("malloc");
		return 1;
	}	
	// initialize all the structs
	for(int i = 0; i < sets*blocks; i++) {
		cache[i].tag = 0;
		cache[i].date = 0;
		cache[i].valid = 0;
	}
	// open tracer file and check for errror
	file = fopen(filename, "rb");
	if (file == NULL) {
		perror("fopen");
		free(cache);
		return 1;
	}
	char *line = NULL;
	size_t len = 0;
	int set, block_index, new_index, lru_search, oldest, hit, p_set, p_hit;
	int mem_acc = 0, hits = 0, misses = 0;
	unsigned long tag, address, prefetch_addr, p_tag;
	// get every address in the tracer
	while(getline(&line, &len, file) != -1) {
		hit = 0;
		mem_acc++; // keep track of total memory accesses
		address = strtoul(line, NULL, 16);
		set = (address / size) % sets; // set the address would be in
		tag = address / (size * sets); // tag the address would have
		// go through each block in the said set and see if any have the said tag
		for (int i = 0; i < blocks; i++) {
			block_index = set * blocks + i;
			if (cache[block_index].valid && cache[block_index].tag == tag) {
				hit = 1;
				cache[block_index].date = mem_acc;
				break;
			}
		} 
		// if not hit, check first for empty blocks in set to place memory in
		if (hit == 0) {
			lru_search = 1;
			for(int i = 0; i < blocks; i++) {
				block_index = set * blocks + i;
				if(cache[block_index].valid == 0) {
					new_index = block_index;
					lru_search = 0;
					break;
				}
			}
			// if no empty blocks, compare block dates in set and remove the oldest (smallest date) block
			if(lru_search == 1) {
				new_index = set*blocks;
				oldest = cache[new_index].date;
				for(int i = 1; i < blocks; i++) {
					block_index = set * blocks + i;
					if(cache[block_index].date < oldest) {
						oldest = cache[block_index].date;
						new_index = block_index;
					}
				}
			}
			// change block metadata
			cache[new_index].tag = tag;
			cache[new_index].valid = 1;
			cache[new_index].date = mem_acc;
			misses++;
		}
		else {hits++;}
		// prefetch option finds the next address and adds it to cache
		if(prefetch == 1) {
			prefetch_addr = address + size;
			p_tag = prefetch_addr / (size * sets); // next tag after one just looked at
			p_set = (prefetch_addr / size) % sets; // next set after one just looked at
			p_hit = 0
			// see if next address is already in block within the set
			for (int i = 0; i < blocks; i++) {
				int idx = p_set * blocks + i;
				if (cache[idx].valid && cache[idx].tag == p_tag) {
					p_hit = 1;
					break;
				}
			}
			// if not, do same as before, check for empty block and add, or replace oldest block
			if(p_hit == 0) {
				lru_search = 1;
				for(int i = 0; i < blocks; i++) {
					block_index = p_set * blocks + i;

					if(cache[block_index].valid == 0) {
						new_index = block_index;
						lru_search = 0;
						break;
					}
				}
				if(lru_search == 1) {
					new_index = p_set*blocks;
					oldest = cache[new_index].date;
					for(int i = 1; i < blocks; i++) {
						block_index = p_set * blocks + i;
						if(cache[block_index].date < oldest) {
							oldest = cache[block_index].date;
							new_index = block_index;
						}
					}
				}
				cache[new_index].tag = p_tag;
				cache[new_index].valid = 1;
				cache[new_index].date = mem_acc;
			}
		}
    }
	// results
	double rate = 100.0 * misses / (misses + hits);
	printf("Accesses: %d\nHits:     %d\nMisses:   %d\nMiss Rate: %.2f%%\n", mem_acc, hits, misses, rate); 
	// clean
	free(cache);
	free(line);
	fclose(file);
	return 0;
}
