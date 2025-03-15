#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <cpu_time> [other args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int cpu_time = atoi(argv[1]);
    printf("Process starting, intended CPU time: %d seconds\n", cpu_time);

    clock_t start = clock();
    while (((double)(clock() - start) / CLOCKS_PER_SEC) < cpu_time) {
        volatile double dummy = 0.0;
        for (int i = 0; i < 1000; i++) {
            dummy += i * i;
        }
    }
    printf("Process finished\n");
    return 0;
}