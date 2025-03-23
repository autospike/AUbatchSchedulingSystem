/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * This file contains the declarations necessary for creating jobs, adding them to queues,
 * and sorting them
 */

#ifndef JOB_H
#define JOB_H

#include <time.h>
#include <pthread.h>

#define JOB_NAME_SIZE 64

typedef enum {
    JOB_WAITING,
    JOB_RUNNING,
    JOB_FINISHED
} job_status_t;

typedef struct job {
    char name[JOB_NAME_SIZE];
    int cpu_time;
    int priority;
    time_t arrival_time;
    time_t start_time;
    time_t finish_time;
    job_status_t status;
    int is_benchmark;
} job_t;

typedef enum {
    POLICY_FCFS,
    POLICY_SJF,
    POLICY_PRIORITY
} scheduling_policy_t;

void add_job(job_t *job);
void list_jobs(void);
job_t* get_next_job(void);
void change_policy(scheduling_policy_t new_policy);
void sort_jobs(void);
static int cmp_fcfs(const void *a, const void *b);
static int cmp_sjf(const void *a, const void *b);
static int cmp_priority(const void *a, const void *b);
int get_job_count(void);
int expected_waiting_time(void);
scheduling_policy_t get_current_policy(void);
//
extern job_t *current_job;
extern pthread_mutex_t job_queue_mutex;
//

#endif