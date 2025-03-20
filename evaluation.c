#include <stdio.h>
#include <stdlib.h>
#include "evaluation.h"
#include "scheduling.h"
#include "job.h"

static int total_jobs = 0;
static double total_turnaround = 0.0;
static double total_cpu = 0.0;
static double total_waiting = 0.0;
static time_t first_job_submission = 0;
static time_t last_job_finish = 0;

void benchmark(const char *benchmark, scheduling_policy_t policy, int num_of_jobs, int priority_levels, int arrival_rate, int min_cpu_time, int max_cpu_time) {
    //Check inputs
    

     //Ensure that there are no current or waiting jobs


    printf("==== Test Metrics ====\nBenchmark Name: %s\nPolicy: %s\nNumber of Jobs: %d\n"
    "Arrival Rate: %f\nPriority Levels: %d\nMin CPU Time: %d\nMax CPU Time: %d\n", 
    benchmark, policy, num_of_jobs, arrival_rate, priority_levels, min_cpu_time, max_cpu_time);    

    printf("Test starting...\n");

    //Set flag to change policy
    set_pending_policy_change(policy);
    
    //loop to wait for policy to be changed
    while (get_current_policy() != policy) {
        sleep(1);
    }

    //Seed the rng
    srand(time(NULL));

    //Submit the jobs
    printf("Adding jobs...\n");

    for (int i = 0; i < num_of_jobs; i++) {
        int cpu_time = rand() % (max_cpu_time - min_cpu_time + 1) + min_cpu_time;
        int priority = rand() % priority_levels + 1;
        submit_job(benchmark, cpu_time, priority);
        sleep(arrival_rate);
    }

    printf("Finished adding jobs."
    "Feel free to add additional jobs with the 'run' command, or view jobs with the 'list' command\n");

    //Wait for all jobs to finish
    while (get_job_count() > 0) {
        sleep(1);
    }

    //Print metrics
    print_performance_metrics();

}

void record_job_evaluation(job_t *job) {
    if (total_jobs == 0) {
        first_job_submission = job->arrival_time;
    }
    total_jobs++;
    double turnaround = difftime(job->finish_time, job->arrival_time);
    double waiting = difftime(job->start_time, job->arrival_time);
    total_turnaround += turnaround;
    total_cpu += job->cpu_time;
    total_waiting += waiting;
    last_job_finish = job->finish_time;
}

void print_performance_metrics() {
    if (total_jobs == 0) {
        printf("No jobs submitted\n");
        return;
    }

    double elapsed = difftime(last_job_finish, first_job_submission);
    double throughput = (elapsed > 0) ? ((double)total_jobs / elapsed) : 0;

    printf("Total number of jobs submitted: %d\n", total_jobs);
    printf("Average turnaround time: %.2f seconds\n",  total_turnaround / total_jobs);
    printf("Average CPU time: %.2f seconds\n", total_cpu / total_jobs);
    printf("Average waiting time: %.2f seconds\n", total_waiting / total_jobs);
    printf("Throughput: %.2f No./second\n", throughput);
}