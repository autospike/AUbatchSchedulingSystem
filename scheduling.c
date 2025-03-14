#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "scheduling.h"

static pthread_t scheduling_thread;
static pthread_t dispatching_thread;

static void *scheduling_thread_func(void *arg) {
    printf("Scheduling\n");
    return NULL;
}

static void *dispatching_thread_func(void *arg) {
    printf("Dispatching\n");
    return NULL;
}

void start_scheduler(void) {
    if (pthread_create(&scheduling_thread, NULL, scheduling_thread_func, NULL) != 0) {
        perror("pthread_create scheduling_thread");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&dispatching_thread, NULL, dispatching_thread_func, NULL) != 0) {
        perror("pthread_create dispatching_thread");
        exit(EXIT_FAILURE); 
    }
}

void stop_scheduler(void) {
    printf("Stopping\n");
    pthread_cancel(scheduling_thread);
    pthread_cancel(dispatching_thread);
    pthread_join(scheduling_thread, NULL);
    pthread_join(dispatching_thread, NULL);
}