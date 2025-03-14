#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "scheduling.h"
#include "job.h"

static pthread_t scheduling_thread;
static pthread_t dispatching_thread;

#define SUBMISSION_QUEUE_SIZE 50
static job_t *submission_queue[SUBMISSION_QUEUE_SIZE];
static int submission_count = 0;
static pthread_mutex_t submission_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t submission_cv = PTHREAD_COND_INITIALIZER;

//Add jobs to temp queue
void submit_job(const char *job_name, int cpu_time, int priority) {
    job_t *new_job = malloc(sizeof(job_t));
    if (!new_job) {
        perror("malloc");
        return;
    }
    strncpy(new_job->name, job_name, JOB_NAME_SIZE);
    new_job->name[JOB_NAME_SIZE - 1] = '\0';
    new_job->cpu_time = cpu_time;
    new_job->priority = priority;

    pthread_mutex_lock(&submission_mutex);
    if (submission_count >= SUBMISSION_QUEUE_SIZE) {
        fprintf(stderr, "Submission queue full, job not accepted\n");
        pthread_mutex_unlock(&submission_mutex);
        free(new_job);
    }
    submission_queue[submission_count++] = new_job;
    pthread_cond_signal(&submission_cv);
    pthread_mutex_unlock(&submission_mutex);
}

//Add jobs to shared queue, then reorder
static void add_job_to_shared_queue(void) {
            pthread_mutex_lock(&submission_mutex);
            while (submission_count == 0) {
                pthread_cond_wait(&submission_cv, &submission_mutex);
            }
            for (int i = 0; i < submission_count; i++) {
                job_t *job = submission_queue[i];
                add_job(job);
                printf("Job %s added to main queue\n", job->name);
            }
            submission_count = 0;
            pthread_mutex_unlock(&submission_mutex);
            
            //reorder_jobs();
}

//Main scheduling thread method, moves jobs from temp to shared queue, reorders jobs
static void *scheduling_thread_func(void *arg) {
    printf("Scheduling\n");
    while(1) {
        add_job_to_shared_queue();
        //maybe return bool so that reorder does not happen twice
        //reorder();
        sleep(1);
    }
    return NULL;
}

//Execute jobs in shared queue
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