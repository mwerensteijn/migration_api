#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <time.h>

#include "trusted_cr.h"

#define NUM_OF_PAGES 50 
#define SMALL_PAGE_SIZE 4096

#define SEC_TO_NANOSEC 1000000000

int main() {
	printf("SRAM Swapping test\n");

	u_int64_t vregs[64];
	unsigned long long starttime;
	unsigned long long stoptime;

	unsigned long long elapsedtime;

	int TEST_BUFFER_SIZE = NUM_OF_PAGES * SMALL_PAGE_SIZE;
	char * test_buffer = (char*) malloc(TEST_BUFFER_SIZE);

	if(!test_buffer) {
		printf("Exit: Unable to allocate %d bytes\n", TEST_BUFFER_SIZE);
		exit(1);
	}

	struct timespec start, end;

	// Test touching multiple pages repeatedly assigning and reading from them causing them to get swapped in and out of SRAM
	// Use this to measure pagefault performance  
	trusted_cr_migrate_to_sw();
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(int x = 0; x < 1000; x++) {
		for(int i = 0; i < TEST_BUFFER_SIZE; i++) {
			test_buffer[i] = 1;
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	starttime = start.tv_sec * SEC_TO_NANOSEC + start.tv_nsec;
	stoptime = end.tv_sec * SEC_TO_NANOSEC + end.tv_nsec;

	elapsedtime = stoptime - starttime;	
	printf("Took: %d seconds and %d nsec\n", elapsedtime / SEC_TO_NANOSEC,  elapsedtime % SEC_TO_NANOSEC);
	
	int p;
	u_int32_t mvfr0;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(int x = 0; x < 1000; x++) {
		for(int i = 0; i < TEST_BUFFER_SIZE; i++) {
			test_buffer[i] = 1;
			// test_buffer[i] = test_buffer[TEST_BUFFER_SIZE-i];
			__asm__ volatile("FADD	S0, S1, S2");
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	starttime = start.tv_sec * SEC_TO_NANOSEC + start.tv_nsec;
	stoptime = end.tv_sec * SEC_TO_NANOSEC + end.tv_nsec;

	elapsedtime = stoptime - starttime;	
	printf("Took: %d seconds and %d nsec\n", elapsedtime / SEC_TO_NANOSEC,  elapsedtime % SEC_TO_NANOSEC);

	clock_gettime(CLOCK_MONOTONIC, &start);
	for(int x = 0; x < 1000; x++) {
		for(int i = 0; i < TEST_BUFFER_SIZE; i++) {
			p = test_buffer[i];
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	starttime = start.tv_sec * SEC_TO_NANOSEC + start.tv_nsec;
	stoptime = end.tv_sec * SEC_TO_NANOSEC + end.tv_nsec;

	elapsedtime = stoptime - starttime;	
	printf("Took: %d seconds and %d nsec\n", elapsedtime / SEC_TO_NANOSEC,  elapsedtime % SEC_TO_NANOSEC);



	trusted_cr_migrate_back_to_nw();

	free(test_buffer);
		
	return 0;
}
