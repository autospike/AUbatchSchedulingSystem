/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * evaluations.c contains the benchmark function,
 * which uses user supplied parameters to create a variable amount of jobs.
 * After these jobs are run, benchmark calls print_benchmark_metrics to print the metrics
 * for said jobs.
 */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "evaluation.h"
#include "scheduling.h"
#include "job.h"

//variables for session metrics
static int total_jobs = 0;
static double total_turnaround = 0.0;
static double total_cpu = 0.0;
static double total_waiting = 0.0;
static time_t first_job_submission = 0;
static time_t last_job_finish = 0;
scheduling_policy_t desired_policy;

//variables for benchmark metrics
static int benchmark_jobs = 0;
static double benchmark_turnaround = 0;
static double benchmark_cpu = 0;
static double benchmark_waiting = 0.0;
static time_t benchmark_first_submission = 0;
static time_t benchmark_last_finish = 0;

//creates numerous jobs based on user input and submits them to be run, then prints out the metrics
void benchmark(const char *benchmark, const char *policy, int num_of_jobs, int priority_levels, double arrival_rate, int min_cpu_time, int max_cpu_time) {
    benchmark_jobs = 0;
    benchmark_turnaround = 0;
    benchmark_cpu = 0;
    benchmark_waiting = 0.0;
    benchmark_last_finish = 0;
    
    //Ensure valid policy
    if (strcmp(policy, "fcfs") == 0) {
        desired_policy = POLICY_FCFS;
    }
    else if (strcmp(policy, "sjf") == 0) {
        desired_policy = POLICY_SJF;
    }
    else if (strcmp(policy, "priority") == 0) {
        desired_policy = POLICY_PRIORITY;
    }
    else {
        printf("Error: Unknown scheduling policy, accepted scheduling policies are: fcfs, sjf, priority\n");
        return;
    }

    //Ensure num inputs are larger than 0
    if (num_of_jobs <= 0 || priority_levels <= 0 || arrival_rate <= 0 || min_cpu_time <= 0 || max_cpu_time <= 0) {
        printf("Error: All numeric paramters must be positive\n");
        return;
    }

    //Ensure max time larger than min time
    if (min_cpu_time > max_cpu_time) {
        printf("Error: max_cpu_time must be equal to or larger than min_cpu_time\n");
        return;
    }

    //Ensure process exists
    if (access(benchmark, F_OK) != 0) {
        printf("Error: Process file does not exist\n");
        return;
    }

    //Ensure that there are no current or waiting jobs
    if (get_job_count() > 0) {
        printf("Please wait until all current jobs are finished before running a benchmark\n");
        return;
    }

    printf("==== Test Metrics ====\nBenchmark Name: %s\nPolicy: %s\nNumber of Jobs: %d\n"
    "Arrival Rate: %f\nPriority Levels: %d\nMin CPU Time: %d\nMax CPU Time: %d\n", 
    benchmark, policy, num_of_jobs, arrival_rate, priority_levels, min_cpu_time, max_cpu_time);    

    printf("\nTest starting...\n");

    //Set flag to change policy
    set_pending_policy_change(desired_policy);
    
    //loop to wait for policy to be changed
    scheduling_policy_t cp = get_current_policy();
    while (get_current_policy() != desired_policy) {
        sleep(1);
    }
    cp = get_current_policy();

    //Seed the rng
    srand(time(NULL));

    //Submit the jobs
    printf("Adding jobs...\n");

    //submit the specified number of jobs, randomly determine their characteristics based on user input
    for (int i = 0; i < num_of_jobs; i++) {
        //list_jobs();
        int cpu_time = rand() % (max_cpu_time - min_cpu_time + 1) + min_cpu_time;
        int priority = rand() % priority_levels + 1;
        submit_job(benchmark, cpu_time, priority, 1);
        usleep((useconds_t)((1.0 / arrival_rate) * 1000000));
    }

    printf("Finished adding jobs.\n");

    //Wait for all jobs to finish
    while (get_job_count() > 0) {
        //list_jobs();
        sleep(1);
    }

    //Print metrics
    print_benchmark_metrics();

}

//record job evaluations to be printed later
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
    //keep benchmark evals seperate
    if (job->is_benchmark) {
        if (benchmark_jobs == 0) {
            benchmark_first_submission = job->arrival_time;
        }
        benchmark_jobs++;
        benchmark_turnaround += turnaround;
        benchmark_cpu += job->cpu_time;
        benchmark_waiting += waiting;
        benchmark_last_finish = job->finish_time;
    }
}

//print session metrics (includes benchmarks)
void print_performance_metrics() {
    if (total_jobs == 0) {
        printf("No jobs completed\n");
        return;
    }

    double elapsed = difftime(last_job_finish, first_job_submission);
    double throughput = (elapsed > 0) ? ((double)total_jobs / elapsed) : 0;
    printf("Overall Metrics\n");
    printf("Total number of jobs submitted: %d\n", total_jobs);
    printf("Average turnaround time: %.2f seconds\n",  total_turnaround / total_jobs);
    printf("Average CPU time: %.2f seconds\n", total_cpu / total_jobs);
    printf("Average waiting time: %.2f seconds\n", total_waiting / total_jobs);
    printf("Throughput: %.2f No./second\n", throughput);
}

//print benchmark metrics only
void print_benchmark_metrics() {
    double benchmark_elapsed = difftime(benchmark_last_finish, benchmark_first_submission);
    double benchmark_throughput = (benchmark_elapsed > 0) ? ((double)benchmark_jobs / benchmark_elapsed) : 0;
    printf("\nBenchmark Metrics:\n");
    printf("Benchmark jobs submitted: %d\n", benchmark_jobs);
    printf("Average turnaround time: %.2f seconds\n",  benchmark_turnaround / benchmark_jobs);
    printf("Average CPU time: %.2f seconds\n", benchmark_cpu/ benchmark_jobs);
    printf("Average waiting time: %.2f seconds\n", benchmark_waiting / benchmark_jobs);
    printf("Throughput: %.2f No./second\n\n", benchmark_throughput);
}
