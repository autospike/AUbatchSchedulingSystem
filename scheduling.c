/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * scheduling.c creates the scheduling and dispatching threads and defines their actions.
 */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "scheduling.h"
#include "job.h"

static pthread_t scheduling_thread;
static pthread_t dispatching_thread;

//Varaibles for intermediate job queue
#define SUBMISSION_QUEUE_SIZE 50
job_t *submission_queue[SUBMISSION_QUEUE_SIZE];
int submission_count = 0;
pthread_mutex_t submission_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t submission_cv = PTHREAD_COND_INITIALIZER;

//Variables for policy changes
static pthread_mutex_t policy_mutex = PTHREAD_MUTEX_INITIALIZER;
static int pending_policy_change = 0;
static scheduling_policy_t pending_policy;

//Add jobs to temp queue so scheduler can see them and add them to dispatch queue
int submit_job(const char *job_name, int cpu_time, int priority, int is_benchmark) {
    
    //Ensure num inputs are larger than 0
    if (cpu_time <= 0 || cpu_time <= 0 || priority <= 0) {
        printf("Error: All numeric paramters must be positive\n");
        return -1;
    }

    //Ensure process exists
    if (access(job_name, F_OK) != 0) {
        printf("Error: Process file does not exist\n");
        return -1;
    }
    
    job_t *new_job = malloc(sizeof(job_t));
    if (!new_job) {
        perror("malloc");
        return -1;
    }
    //Set job parameters based on user inputs
    strncpy(new_job->name, job_name, JOB_NAME_SIZE);
    new_job->name[JOB_NAME_SIZE - 1] = '\0';
    new_job->cpu_time = cpu_time;
    new_job->priority = priority;
    new_job->is_benchmark = is_benchmark;
    
    
    //Checks if the dispatch queue is full
    pthread_mutex_lock(&submission_mutex);
    if (submission_count >= SUBMISSION_QUEUE_SIZE) {
        fprintf(stderr, "Submission queue full, job not accepted\n");
        pthread_mutex_unlock(&submission_mutex);
        free(new_job);
        return -1;
    }
    submission_queue[submission_count++] = new_job;
    pthread_cond_signal(&submission_cv);
    pthread_mutex_unlock(&submission_mutex);
    return 0;
}

//Add jobs to dispatch queue, then reorder
static void add_job_to_shared_queue(void) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;

            pthread_mutex_lock(&submission_mutex);
            while (submission_count == 0) {
                int ret = pthread_cond_timedwait(&submission_cv, &submission_mutex, &ts);
                if (ret == ETIMEDOUT) {
                    break;
                }
            }

            for (int i = 0; i < submission_count; i++) {
                job_t *job = submission_queue[i];
                add_job(job);
            }
            submission_count = 0;
            pthread_mutex_unlock(&submission_mutex);
            //call function to sort dispatch queue
            sort_jobs();
}

//Main scheduling thread method, moves jobs from temp to dispatch queue, reorders jobs
static void *scheduling_thread_func(void *arg) {
    //continuously looks for new policies and jobs
    while(1) {
        //Call function to add job to dispatch queue
        add_job_to_shared_queue();
        pthread_mutex_lock(&policy_mutex);
        //switch policies if user has requested a new one
        if (pending_policy_change) {
            change_policy(pending_policy);
            printf("Scheduling policy is switched to ");
            switch(pending_policy) {
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
            //if there are jobs waiting, let user know they have been rescheduled
            if (get_job_count() > 1) {
                printf("All %d waiting jobs have been rescheduled\n", get_job_count() -1);
            }
            pending_policy_change = 0;
        }
        pthread_mutex_unlock(&policy_mutex);
        sleep(1);
    }
    return NULL;
}

//Main dispatching thread, executes jobs in dispatch queue
static void *dispatching_thread_func(void *arg) {
    //continuously checks for new jobs in the dispatch queue
    while (1) {
        job_t *job = get_next_job();
        if (job == NULL) {
            sleep(1);
            continue;
        }
        pthread_mutex_lock(&job_queue_mutex);
        current_job = job;
        job->start_time = time(NULL);
        job->status = JOB_RUNNING;
        pthread_mutex_unlock(&job_queue_mutex);

        pid_t pid = fork();
        //if fork fails
        if (pid < 0) {
            perror("fork failed");
            pthread_mutex_lock(&job_queue_mutex);
            current_job = NULL;
            pthread_mutex_unlock(&job_queue_mutex);
            free(job);
            continue;
        }
        //child executes job
        else if (pid == 0) {
            char cpu_time_str[16];
            char priority_str[16];
            snprintf(cpu_time_str, sizeof(cpu_time_str), "%d", job->cpu_time);
            snprintf(priority_str, sizeof(priority_str), "%d", job->priority);
            char *args[4];
            args[0] = job->name;
            args[1] = cpu_time_str;
            args[2] = priority_str;
            args[3] = NULL;

            execv(job->name, args);
            perror("execv failed");
            exit(EXIT_FAILURE);
        }
        else {
            int status;
            waitpid(pid, &status, 0);
            pthread_mutex_lock(&job_queue_mutex);
            job->finish_time = time(NULL);
            job->status = JOB_FINISHED;
            current_job = NULL;
            pthread_mutex_unlock(&job_queue_mutex);
            //record job stats to be printed later
            record_job_evaluation(job);
            free(job);
        }
        sleep(1);
    }
    return NULL;
}

//create scheduler and dispatcher threads, point them to their respective functions
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

//stop threads
void stop_scheduler(void) {
    pthread_cancel(scheduling_thread);
    pthread_cancel(dispatching_thread);
    pthread_join(scheduling_thread, NULL);
    pthread_join(dispatching_thread, NULL);
}

//set pending policy so that the scheduler knows the change it
void set_pending_policy_change(scheduling_policy_t new_policy) {
    pthread_mutex_lock(&policy_mutex);
    pending_policy_change = 1;
    pending_policy = new_policy;
    pthread_mutex_unlock(&policy_mutex);
}
