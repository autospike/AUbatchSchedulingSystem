#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "job.h"
#include "scheduling.h"

#define MAX_JOBS 100

job_t *job_queue[MAX_JOBS];
static scheduling_policy_t current_policy = POLICY_FCFS;
int job_count = 0;
pthread_mutex_t job_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
job_t *current_job = NULL;

void add_job(job_t *job) {
    pthread_mutex_lock(&job_queue_mutex);

    if (job_count >= MAX_JOBS) {
        perror("Job queue is full\n");
        free(job);
        pthread_mutex_unlock(&job_queue_mutex);
        return;
    }
    job->arrival_time = time(NULL);
    job->status = JOB_WAITING;
    job_queue[job_count++] = job;
    sort_jobs();
    
    pthread_mutex_unlock(&job_queue_mutex);
}

void list_jobs(void) {
    int count = get_job_count();
    pthread_mutex_lock(&job_queue_mutex);
    printf("Total number of jobs in queue: %d\n", count);
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

    if (current_job != NULL) {
        char time_str[9];
        struct tm *tm_info = localtime(&current_job->arrival_time);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
        
        int elapsed = (int)(time(NULL) - current_job->start_time);
        int remaining = current_job->cpu_time - elapsed;
        if (remaining < 0) {
            remaining = 0;
        }

        printf("%-15s %10d %5d %15s %10s\n", current_job->name, remaining, current_job->priority, time_str, "Running");
    }

    for (int i = 0; i < job_count; i++) {
        char time_str[9];
        struct tm *tm_info = localtime(&job_queue[i]->arrival_time);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
        
        char progress[10] = "";
        if (job_queue[i]->status == JOB_WAITING) {
            snprintf(progress, sizeof(progress), "Waiting");
        }

        printf("%-15s %10d %5d %15s %10s\n", job_queue[i]->name, job_queue[i]->cpu_time, job_queue[i]->priority, time_str, progress);
    }
    printf("\n");
    pthread_mutex_unlock(&job_queue_mutex);
}

job_t* get_next_job(void) {
    pthread_mutex_lock(&job_queue_mutex);
    if (job_count == 0) {
        pthread_mutex_unlock(&job_queue_mutex);
        return(NULL);
    }
    job_t *job = job_queue[0];
    for (int i = 1; i < job_count; i++) {
        job_queue[i - 1] = job_queue[i];
    }
    job_count--;
    pthread_mutex_unlock(&job_queue_mutex);
    return job;
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

//Comparison functions for qsort()
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
    return jobB->priority - jobA->priority;
}

int get_job_count(void) {
    int count;
    pthread_mutex_lock(&job_queue_mutex);
    count = job_count;

    if (current_job != NULL) {
        count++;
    }

    pthread_mutex_unlock(&job_queue_mutex);
    
    pthread_mutex_lock(&submission_mutex);
    count += submission_count;
    pthread_mutex_unlock(&submission_mutex);
    return count;
}

int expected_waiting_time(void) {
    int total = 0;
    time_t now = time(NULL);

    pthread_mutex_lock(&job_queue_mutex);
    for (int i = 0; i < job_count; i++) {
        if (job_queue[i]->status == JOB_WAITING) {
            total += job_queue[i]->cpu_time;
        }
    }
    
    if (current_job != NULL && current_job->status == JOB_RUNNING) {
        int elapsed = (int)(now - current_job->start_time);
        int remaining = current_job->cpu_time - elapsed;
        if (remaining < 0) {
            remaining = 0;
        }
        total += remaining;
    }
    
    pthread_mutex_unlock(&job_queue_mutex);

    pthread_mutex_lock(&submission_mutex);
    for (int i = 0; i < submission_count; i++) {
        total += submission_queue[i]->cpu_time;
    }
    pthread_mutex_unlock(&submission_mutex);
    return total;
}

scheduling_policy_t get_current_policy(void) {
    scheduling_policy_t cp;
    pthread_mutex_lock(&job_queue_mutex);
    cp = current_policy;
    pthread_mutex_unlock(&job_queue_mutex);
    return cp;
}
