#ifndef EVALUATION_H
#define EVALUATION_H

#include "job.h"

void benchmark(const char *benchmark, const char *policy, int num_of_jobs, int priority_levels, double arrival_rate, int min_cpu_time, int max_cpu_time);
void record_job_evaluation(job_t *job);
void print_performance_metrics();

#endif