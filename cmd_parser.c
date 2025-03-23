/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * cmd_parser runs a continuous loop that reads user input and calls process_command() to process it
 * The various user commands carry out the desired function by either calling other function directly, 
 * or by storing data so that the scheduler thread can read it later.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "cmd_parser.h"
#include "scheduling.h"
#include "job.h"

//Continuously listen for user commands
void cmd_parser() {
    char *line = NULL;
    size_t len = 0;
    
    //print welcome
    printf("Welcome to William Baker's batch job schedular Version 1.0\n");
    printf("Type 'help' to find more about AUbatch commands.\n");
    
    //listen for commands
    while (1) {
        printf("> ");
        if (getline(&line, &len, stdin) != -1) {
            //Call function for processing commands
            process_command(line);
        }
    }
    free(line);
}

//Process commands
static void process_command(char *line) {
    line[strcspn(line, "\n")] = '\0';

    char command[16];

    if (sscanf(line, "%15s", command) != 1) {
        return;
    }
    //help command, prints out menu
    if (strcmp(command, "help") == 0) {
        help_menu();
        return;
    }
    //run command, submits jobs to be run
    else if (strcmp(command, "run") == 0) {
        char job_name[64];
        int cpu_time;
        int priority;
        if (sscanf(line, "run %63s %d %d", job_name, &cpu_time, &priority) != 3) {
            printf("Usage: run <job> <time> <priority>\n");
            return;
        }
        //call function submit job to temporary queue so that the scheduler can place it in the dispatch queue
        if (submit_job(job_name, cpu_time, priority, 0) != 0) {
            return;
        }
        printf("Job %s was submitted.\n", job_name);
        printf("Total number of jobs in the queue: %d\n", get_job_count());
        printf("expected waiting time: %d seconds\n", expected_waiting_time());
        printf("Scheduling Policy: ");
        scheduling_policy_t cp = get_current_policy();
        switch(cp) {
            case POLICY_FCFS:
                    printf("FCFS.\n\n");
                    break;
                case POLICY_SJF:
                    printf("SJF.\n\n");
                    break;
                case POLICY_PRIORITY:
                    printf("Priority.\n\n");
                    break;
                default:
                    printf("FCFS.\n\n");
                    break;
        }
        return;
    }
    //Prints out list of scheduled and running jobs
    else if (strcmp(command, "list") == 0) {
        //Call funciton to print out scheduled and running jobs
        list_jobs();
        return;
    }
    //Set policy to fcfs
    else if (strcmp(command, "fcfs") == 0) {
        //Call function to set policy to fcfs so that the scheduler updates the queue
        set_pending_policy_change(POLICY_FCFS);
        return;
    }
    //Set policy to sjf
    else if (strcmp(command, "sjf") == 0) {
        //Call function to set policy to sjf so that the scheduler updates the queue
        set_pending_policy_change(POLICY_SJF);
        return;
    }
    //Set policy to priority
    else if (strcmp(command, "priority") == 0) {
        //Call function to set policy to priority so that the scheduler updates the queue
        set_pending_policy_change(POLICY_PRIORITY);
        return; 
    }
    //Run a benchmark
    else if (strcmp(command, "test") == 0) {
        char benchmark_str[64];
        char policy[16];
        int num_of_jobs, priority_levels, min_cpu_time, max_cpu_time;
        double arrival_rate;
        if (sscanf(line, "test %63s %15s %d %d %lf %d %d", benchmark_str, policy, &num_of_jobs, &priority_levels, &arrival_rate, &min_cpu_time, &max_cpu_time) !=7) {
            printf("Usage: test <job> <policy> <num_of_jobs> <priority_levels> <arrival_rate> <min_CPU_time> <max_CPU_time>\n");
            return;
        }
        //Call the benchmark function
        benchmark(benchmark_str, policy, num_of_jobs, priority_levels, arrival_rate, min_cpu_time, max_cpu_time);
        return;
    }
    //Print session metrics and exit
    else if (strcmp(command, "quit") == 0) {
        //Wait for all jobs to finish
        if (get_job_count() > 0) {
            printf("Waiting for all jobs to finish before exiting...\n");
        }
        while (get_job_count() > 0) {
            sleep(1);
        }
        //Call funciton to stop the sceduler and dispatcher threads
        stop_scheduler();
        //Call function to print session metrics
        print_performance_metrics();
        //Exit
        exit(0);
    }
    //Handle unknown commands
    else {
        printf("Error: Unknown command\n"); 
    }
}

//data type used to represent commands and dscriptions on the help menu
typedef struct {
    const char *command;
    const char *description;
} menu_item;

//content of the help menu
menu_item menu[] = {
    {"run <job> <time> <pri>","submit a job named <job>,\n                            execution time is <time>,\n                            priority time is <pri>"},
    {"list","display the job status"},
    {"fcfs","change the scheduling policy to FCFS"},
    {"sjf","change the sheduling policy to SJF"},
    {"priority","change the sheduling policy to priority"},
    {"test <benchmark> <policy> <num_of_jobs> <priority_levels>\n     <min_CPU_time> <max_CPU_time>",""},
    {"quit","exit AUbatch"},
    {NULL, NULL}
};

//prints help menu
static void help_menu() {
    for (int i = 0; menu[i].command != NULL; i++) {
        if (menu[i].description == NULL || menu[i].description[0] == '\0') {
            printf("%-25s\n", menu[i].command);
        }
        else {
            printf("%-25s : %s\n", menu[i].command, menu[i].description);
        }
    }
}
