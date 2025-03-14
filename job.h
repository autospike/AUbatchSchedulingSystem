#ifndef JOB_H
#define JOB_H

#include <time.h>

#define JOB_NAME_SIZE 64

void add_job(job_t *job);

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

#endif