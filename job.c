#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "job.h"

#define MAX_JOBS 100

static job_t *job_queue[MAX_JOBS];
static scheduling_policy_t current_policy = POLICY_FCFS;
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
    sort_jobs();
    
    pthread_mutex_unlock(&job_queue_mutex);
}

void list_jobs(void) {
    pthread_mutex_lock(&job_queue_mutex);
    
    printf("Total number of jobs in queue: %d\n", job_count);
    printf("Scheduling Policy: ");
    switch(current_policy) {
        case POLICY_FCFS:
            printf("FCFS.\n");
            break;
        case POLICY_SJF:
            printf("SJF.\n");
            break;
        case POLICY_PRIORITY:
            printf("Priority.\n");
            break;
        default:
            printf("FCFS.\n");
            break;
    }

    printf("%-15s %10s %5s %15s %10s\n","Name", "CPU_Time", "Pri", "Arrival_time", "Progress");
    for (int i = 0; i < job_count; i++) {
        char time_str[9];
        struct tm *tm_info = localtime(&job_queue[i]->arrival_time);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

        char progress[10] = "";
        if (job_queue[i]->status == JOB_RUNNING) {
            snprintf(progress, sizeof(progress), "Run");
        }

        printf("%-15s %10d %5d %15s %10s\n", job_queue[i]->name, job_queue[i]->cpu_time, job_queue[i]->priority, time_str, progress);
    }
    pthread_mutex_unlock(&job_queue_mutex);
}

void change_policy(scheduling_policy_t new_policy) {
    pthread_mutex_lock(&job_queue_mutex);
    current_policy = new_policy;
    sort_jobs();
    pthread_mutex_unlock(&job_queue_mutex);
}

void sort_jobs(void) {
    if (job_count <= 1)
        return;
    switch (current_policy) {
        case POLICY_FCFS:
            qsort(job_queue, job_count, sizeof(job_t*), cmp_fcfs);
            break;
        case POLICY_SJF:
            qsort(job_queue, job_count, sizeof(job_t*), cmp_sjf);
            break;
        case POLICY_PRIORITY:
            qsort(job_queue, job_count, sizeof(job_t*), cmp_priority);
            break;
    }
}

/* Comparison functions for qsort() */
static int cmp_fcfs(const void *a, const void *b) {
    job_t *jobA = *(job_t **)a;
    job_t *jobB = *(job_t **)b;
    return (int)(jobA->arrival_time - jobB->arrival_time);
}

static int cmp_sjf(const void *a, const void *b) {
    job_t *jobA = *(job_t **)a;
    job_t *jobB = *(job_t **)b;
    return jobA->cpu_time - jobB->cpu_time;
}

static int cmp_priority(const void *a, const void *b) {
    job_t *jobA = *(job_t **)a;
    job_t *jobB = *(job_t **)b;
    if (jobA->priority == jobB->priority)
        return cmp_fcfs(a, b);
    return jobA->priority - jobB->priority;
}