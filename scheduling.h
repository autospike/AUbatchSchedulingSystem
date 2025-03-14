#ifndef SCHEDULING_H
#define SCHEDULING_H

void scheduling(void);
static void *scheduling_thread_func(void *arg);
static void *dispatching_thread_func(void *arg);
void start_scheduler(void);
void stop_scheduler(void);


#endif