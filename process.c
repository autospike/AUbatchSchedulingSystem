/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * process.c is the process/job that I use with my AUbatch program
 * 
 * This program was compiled on its own with the following command:
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <cpu_time> [other args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int cpu_time = atoi(argv[1]);
    //do meaningless work
    clock_t start = clock();
    while (((double)(clock() - start) / CLOCKS_PER_SEC) < cpu_time) {
        volatile double dummy = 0.0;
        for (int i = 0; i < 1000; i++) {
            dummy += i * i;
        }
    }
    return 0;
}