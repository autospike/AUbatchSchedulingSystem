#ifndef SCHEDULING_H
#define SCHEDULING_H

void scheduling(void);
void submit_job(const char *job_name, int cpu_time, int priority);
static void add_job_to_shared_queue(void);
static void *scheduling_thread_func(void *arg);
static void *dispatching_thread_func(void *arg);
void start_scheduler(void);
void stop_scheduler(void);


#endif