#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "job.h"

#define MAX_JOBS 100

static job_t *job_queue[MAX_JOBS];
int job_count = 0;

static pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_job(job_t *job) {
    pthread_mutex_lock(&job_queue_mutex);
    if (job_count >= MAX_JOBS) {
        perror("Job queue is full\n");
        free(job);
    }
    job->arrival_time = time(NULL);
    job->status = JOB_WAITING;
    job_queue[job_count++] = job;
    //sort_jobs();
    printf("Job added\n");
    pthread_mutex_unlock(&job_queue_mutex);
}