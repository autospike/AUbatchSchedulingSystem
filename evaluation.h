#ifndef EVALUATION_H
#define EVALUATION_H

void benchmark(const char *benchmark, scheduling_policy_t policy, int num_of_jobs, int priority_levels, int arrival_rate, int min_cpu_time, int max_cpu_time);
void record_job_evaluation(job_t *job);
void print_performance_metrics();

#endif