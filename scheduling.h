/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * This file contains the declarations necessary for creating the scheduler and dipatcher threads, 
 * adding jobs to queues, and changing scheduling policies
 */

#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "job.h"

#define SUBMISSION_QUEUE_SIZE 50

extern job_t *submission_queue[SUBMISSION_QUEUE_SIZE];
extern int submission_count;
extern pthread_mutex_t submission_mutex;
extern pthread_cond_t submission_cv;

void scheduling(void);
int submit_job(const char *job_name, int cpu_time, int priority, int is_benchmark);
static void add_job_to_shared_queue(void);
static void *scheduling_thread_func(void *arg);
static void *dispatching_thread_func(void *arg);
void start_scheduler(void);
void stop_scheduler(void);
void set_pending_policy_change(scheduling_policy_t new_policy);

#endif