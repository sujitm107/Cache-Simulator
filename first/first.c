#include <stdlib.h>
#include <stdio.h>
#include <math.h>
//./first <cache size><block size><cache policy><associativity><prefetch size><trace file>
struct cacheBlock{

	unsigned int valid;
	unsigned long int tag;
	unsigned int time;
	
};

int search_cache(unsigned int set_index, unsigned long tag, struct cacheBlock** cache);

int search_cache(unsigned int set_index, unsigned long tag, struct cacheBlock** cache){
	printf("tag: %lu,  set Index: %u\n", tag, set_index);

	if(cache[set_index]->tag != tag){
		cache[set_index]->tag = tag;
		cache[set_index]->valid = 1;
		return -1;
	} else{
		return 1;
	}

	return 0;
}


int main(int argc, char** argv){

	int num_reads = 0;
	int num_writes = 0;
	int num_cache_hits = 0;
	int num_cache_misses = 0;

	int cache_size = atoi(argv[1]);
	int block_size = atoi(argv[2]);
	//char* cache_polity = argv[3];
	int associativity = 1; //hardcoding to one bc for direct associativity is 1
	//int prefetch_size = argv[5]; 

	int valid_inputs = 0;
	for(int i = 0; i<cache_size; i++){
		if(i == log2(cache_size)){
			valid_inputs++;
		}
		if(i == log2(block_size)){
			valid_inputs++;
		}
	}
	
	if(valid_inputs < 2){
		return 0;
	}

	int num_sets = cache_size/(associativity*block_size); //could use '1<<set_index_bits' instead

	int offset_bits = log2(block_size);
	int set_index_bits = log2(num_sets); //taking the log of the number of sets
	int tag_bits = 48 - offset_bits - set_index_bits;

	int mask = (1 << set_index_bits) - 1;

	printf("Offset Bits: %d\n", offset_bits);
	printf("Index Bits: %d\n", set_index_bits);
	printf("Tag Bits: %d\n", tag_bits);
	printf("Number of Sets: %d\n", num_sets);

//ALLOCATING CACHE
	struct cacheBlock** cache = malloc(num_sets*sizeof(struct cacheBlock*));
	for(int i = 0; i<num_sets; i++){
		cache[i] = malloc(associativity*sizeof(struct cacheBlock));
	}


	FILE* fp = fopen(argv[6], "r");
	unsigned long long int address;
	char command;
	while(fscanf(fp, "%c %llx\n", &command, &address) > 0){
		if(command == '#'){
			break;
		}
		if(command == 'W'){
			num_writes += 1;
			
		}
		unsigned int set_index = (address >> offset_bits) & mask;
		unsigned long tag = (address >> offset_bits) >> set_index_bits;

		if(search_cache(set_index, tag, cache) == 1){
			num_cache_hits++;
		} else{
			num_cache_misses++;
			num_reads++;
		}
		
	}
	
	printf("Memory reads: %d\n", num_reads);
	printf("Memory writes: %d\n", num_writes);
	printf("Cache hits: %d\n", num_cache_hits);
	printf("Cache misses: %d\n", num_cache_misses);
	
	return 0;
}