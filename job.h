#ifndef JOB_H
#define JOB_H

#include <time.h>

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
} job_t;

typedef enum {
    POLICY_FCFS,
    POLICY_SJF,
    POLICY_PRIORITY
} scheduling_policy_t;

void add_job(job_t *job);
void list_jobs(void);
void change_policy(scheduling_policy_t new_policy);
void sort_jobs(void);
static int cmp_fcfs(const void *a, const void *b);
static int cmp_sjf(const void *a, const void *b);
static int cmp_priority(const void *a, const void *b);

#endif