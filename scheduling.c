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

//add testing
//fix ">" print formatting
//input checks (process name, job params)

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
        return;
    }
    submission_queue[submission_count++] = new_job;
    pthread_cond_signal(&submission_cv);
    pthread_mutex_unlock(&submission_mutex);
}

//Add jobs to shared queue, then reorder
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
            sort_jobs();
}

//Main scheduling thread method, moves jobs from temp to shared queue, reorders jobs
static void *scheduling_thread_func(void *arg) {
    while(1) {
        add_job_to_shared_queue();
        pthread_mutex_lock(&policy_mutex);
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
            //use real number
            if (get_job_count() > 1) {
                printf("All the %d waiting jobs have been rescheduled\n", get_job_count() -1);
            }
            pending_policy_change = 0;
        }
        pthread_mutex_unlock(&policy_mutex);
        sleep(1);
    }
    return NULL;
}

//Execute jobs in shared queue
static void *dispatching_thread_func(void *arg) {
    while (1) {
        job_t *job = get_next_job();
        if (job == NULL) {
            sleep(1);
            continue;
        }
        //
        pthread_mutex_lock(&job_queue_mutex);
        current_job = job;
        //
        job->start_time = time(NULL);
        job->status = JOB_RUNNING;
        //
        pthread_mutex_unlock(&job_queue_mutex);
        //

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            //
            pthread_mutex_lock(&job_queue_mutex);
            current_job = NULL;
            pthread_mutex_unlock(&job_queue_mutex);
            //
            free(job);
            continue;
        }
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
            //
            pthread_mutex_lock(&job_queue_mutex);
            //
            job->finish_time = time(NULL);
            job->status = JOB_FINISHED;
            //
            current_job = NULL;
            pthread_mutex_unlock(&job_queue_mutex);
            //
            record_job_evaluation(job);
            free(job);
        }
        sleep(1);
    }
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
    pthread_cancel(scheduling_thread);
    pthread_cancel(dispatching_thread);
    pthread_join(scheduling_thread, NULL);
    pthread_join(dispatching_thread, NULL);
}

void set_pending_policy_change(scheduling_policy_t new_policy) {
    pthread_mutex_lock(&policy_mutex);
    pending_policy_change = 1;
    pending_policy = new_policy;
    pthread_mutex_unlock(&policy_mutex);
}