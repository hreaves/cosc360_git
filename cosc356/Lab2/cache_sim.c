#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned long tag;
	int date;
	int valid;
} Block;

int main(int argc, char **argv) {
	
	int sets, blocks, size, prefetch = 0;
	FILE *file;
	char *filename;

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
	if (sets <= 0 || blocks <= 0 || size <= 0 || filename == NULL) {
		fprintf(stderr, "Usage: %s --sets <num> --blocks <num> --size <num> --trace <file>\n", argv[0]);
		return 1;
	}

	Block *cache = malloc(sets*blocks*sizeof(Block));
	if (cache == NULL) {
		perror("malloc");
		return 1;
	}	
	for(int i = 0; i < sets*blocks; i++) {
		cache[i].tag = 0;
		cache[i].date = 0;
		cache[i].valid = 0;
	}
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
	while(getline(&line, &len, file) != -1) {
		hit = 0;
		mem_acc++;
		address = strtoul(line, NULL, 16);
		set = (address / size) % sets;
		tag = address / (size * sets);
		for (int i = 0; i < blocks; i++) {
			block_index = set * blocks + i;
			if (cache[block_index].valid && cache[block_index].tag == tag) {
				hit = 1;
				cache[block_index].date = mem_acc;
				break;
			}
		} 
		
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
			cache[new_index].tag = tag;
			cache[new_index].valid = 1;
			cache[new_index].date = mem_acc;
			misses++;
		}
		else {hits++;}
		
		if(prefetch == 1) {
			prefetch_addr = address + size;
			p_tag = prefetch_addr / (size * sets);
			p_set = (prefetch_addr / size) % sets;
			p_hit = 0;

			for (int i = 0; i < blocks; i++) {
				int idx = p_set * blocks + i;
				if (cache[idx].valid && cache[idx].tag == p_tag) {
					p_hit = 1;
					break;
				}
			}
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
	double rate = 100.0 * misses / (misses + hits);
	printf("Accesses: %d\nHits:     %d\nMisses:   %d\nMiss Rate: %.2f%%\n", mem_acc, hits, misses, rate); 
	free(cache);
	free(line);
	fclose(file);
	return 0;
}
