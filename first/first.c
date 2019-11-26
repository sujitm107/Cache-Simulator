#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
//./first <cache size><block size><cache policy><associativity><prefetch size><trace file>
struct cacheBlock{

	int valid;
	unsigned long int tag;
	unsigned int time;
	
};

int checkHit(struct cacheBlock** set, unsigned long tag, int associativity);
void writeToCache(struct cacheBlock** set, unsigned long tag, int associativity);
unsigned int getMaxTime(struct cacheBlock** set, int associativity);
//void printArray(struct cacheBlock** cache, int num_sets, int associativity);

unsigned int getMaxTime(struct cacheBlock** set, int associativity){
	unsigned int maxTime = 0;
	for(int i = 0; i<associativity; i++){
		if(set[i]->valid == 1){

			if(maxTime < set[i]->time){
				maxTime = set[i]->time;
			}
		}
	}

	return maxTime;
}

int checkHit(struct cacheBlock** set, unsigned long tag, int associativity){
	//set[0]->tag = tag;
	//printf("%lu\n", set[0]->tag);

	for(int i = 0; i<associativity; i++){
		//printf("set: %lu, tagCompared: %lu ", set[i]->tag, tag);
			if(set[i]->valid != 1){
				return 0;
			}

			if(set[i]->tag == tag){
				return 1;
			}
	}

	return 0;
}

void writeToCache(struct cacheBlock** set, unsigned long tag, int associativity){

	struct cacheBlock* temp = malloc(sizeof(struct cacheBlock));
	temp->tag = tag;
	unsigned int time = getMaxTime(set, associativity); //returns the highest time
	temp->time = time+1;

	int i = 0;
	unsigned int minTime = time;
	//printf("HUGE_VAL: %u\n", minTime);
	int minIndex = 0;

	for(i = 0; i<associativity; i++){
		if(set[i]->valid != 1){
			set[i] = temp;
			set[i]->valid = 1;
			//printf("Inserted in empty slot\n");
			return;
		}
		if(minTime > set[i]->time){
			minTime = set[i]->time;
			minIndex = i;
		}
	}

//LAST RESORT
	//set[0] = temp;
	//printf("putting in %lu\n", tag);

//FULL
	if(i == associativity){ //returns an index
		set[minIndex] = temp;
		temp->valid = 1;
		//printf("Inserted through replacement\n");
	}

}

//START OF MAIN
int main(int argc, char** argv){

	int cache_size = atoi(argv[1]);
	int block_size = atoi(argv[2]);
	char* cache_policy = argv[3];
	int associativity = 1;
	if (strcmp(argv[4],"direct") == 0){	
		associativity = 1;
	}else if(strcmp(argv[4], "assoc") == 0){
		associativity = cache_size/block_size;
	}else{
		associativity = argv[4][6] - '0';
		for(int i = 0; i<associativity; i++){
			if(i == log2(associativity)){
				break;
			}
			if(i == associativity-1){
				return 0;
			}
		}
	}

	int prefetch_size = atoi(argv[5]); 

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
		printf("error\n");
		return 0;
	}

	int num_sets = cache_size/(associativity*block_size); //could use '1<<set_index_bits' instead

	int offset_bits = log2(block_size);
	int set_index_bits = log2(num_sets); //taking the log of the number of sets
	//int tag_bits = 48 - offset_bits - set_index_bits;

	int mask = (1 << set_index_bits) - 1;

	// printf("Offset Bits: %d\n", offset_bits);
	// printf("Index Bits: %d\n", set_index_bits);
	// printf("Tag Bits: %d\n", tag_bits);
	// printf("Number of Sets: %d\n", num_sets);
	// printf("Prefetch Size: %d\n", prefetch_size);
	// printf("Associativity: %d\n", associativity);
	printf("%s\n", cache_policy);

//ALLOCATING CACHE
	for(int k = 0; k<2; k++){
		int num_reads = 0;
		int num_writes = 0;
		int num_cache_hits = 0;
		int num_cache_misses = 0;

		struct cacheBlock*** cache = malloc(num_sets*sizeof(struct cacheBlock**));

		for(int r = 0; r<num_sets; r++){
			cache[r] = malloc(associativity*sizeof(struct cacheBlock*));
				for(int i = 0; i<associativity; i++){
					cache[r][i] = malloc(sizeof(struct cacheBlock));
				}
		}

		FILE* fp = fopen(argv[6], "r");
		unsigned long long int address;
		char command;
		while(fscanf(fp, "%c %llx\n", &command, &address) > 0){
			if(command == '#'){
				break;
			}

			unsigned int set_index = (address >> offset_bits) & mask;
			unsigned long tag = (address >> offset_bits) >> set_index_bits;

			// if(command == 'W'){
			// 	num_writes += 1;
				
			// }
			//printf("%d\n", set_index);
			if(k == 0){ //WOUT PREFETCH
				if(checkHit(cache[set_index], tag, associativity) == 1){
					//printf("hit\n");

					num_cache_hits++;
					if(command == 'W'){
						num_writes++;
					}
				} else {
					//printf("miss\n");

					writeToCache(cache[set_index], tag, associativity);
					num_cache_misses++;
					if(command == 'W'){
						num_reads++;
						num_writes++;
					} else {
						num_reads++;
					}
				}
			 } else{ //PREFETCH
				if(checkHit(cache[set_index], tag, associativity) == 1){
					num_cache_hits++;
					if(command == 'W'){
						num_writes++;
					}
				} else {
					//printf("%llu miss\n", address);
					writeToCache(cache[set_index], tag, associativity);
					num_cache_misses++;
					if(command == 'W'){
						num_reads++;
						num_writes++;
					} else {
						num_reads++;
					}

					unsigned long long int prefetch_address = address;
					for(int i = 0; i<prefetch_size; i++){
						prefetch_address = prefetch_address + block_size;
						//printf("%llx\n", prefetch_address);
						set_index = (prefetch_address >> offset_bits) & mask;
						tag = (prefetch_address >> offset_bits) >> set_index_bits;
						if(checkHit(cache[set_index], tag, associativity) == 0){
							writeToCache(cache[set_index], tag, associativity);
							num_reads++;
						}
					}
				}
			}
			
		}

		if(k == 0){
			printf("no-prefetch\n");
		}
		else{
			printf("with-prefetch\n");
		}
		printf("Memory reads: %d\n", num_reads);
		printf("Memory writes: %d\n", num_writes);
		printf("Cache hits: %d\n", num_cache_hits);
		printf("Cache misses: %d\n", num_cache_misses);

	}
	
	
	// printf("with-prefetch\n");
	// printf("Memory reads: %d\n", 0);
	// printf("Memory writes: %d\n", 0);
	// printf("Cache hits: %d\n", 0);
	// printf("Cache misses: %d\n", 0);

	//printArray(cache, num_sets, associativity);
	
	return 0;
}

// void printArray(struct cacheBlock** cache, int num_sets, int associativity){

// 	for(int i = 0; i<num_sets; i++){
// 		for(int j = 0; j<associativity; j++){
// 			printf("Tag: %lu Valid: %d Time: %u\n", cache[i]->tag, cache[i]->valid, cache[i]->time);
// 		}
// 		printf("\n");
// 	}

// }