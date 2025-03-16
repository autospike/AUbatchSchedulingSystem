#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "cmd_parser.h"
#include "scheduling.h"
#include "job.h"

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
            process_command(line);
        }
    }
    free(line);
}

static void process_command(char *line) {
    line[strcspn(line, "\n")] = '\0';

    char command[16];

    if (sscanf(line, "%15s", command) != 1) {
        return;
    }

    if (strcmp(command, "help") == 0) {
        help_menu();
        return;
    }
    else if (strcmp(command, "run") == 0) {
        char job_name[64];
        int cpu_time;
        int priority;
        if (sscanf(line, "run %63s %d %d", job_name, &cpu_time, &priority) != 3) {
            printf("Usage: run <job> <time> <priority>\n");
            return;
        }
        submit_job(job_name, cpu_time, priority);
        printf("Job %s was submitted.\n", job_name);
        printf("Total number of jobs in the queue: %d\n", get_job_count());
        printf("expected waiting time: %d seconds\n", expected_waiting_time());
        printf("Scheduling Policy: ");
        scheduling_policy_t cp = get_current_policy();
        switch(cp) {
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
        return;
    }
    else if (strcmp(command, "list") == 0) {
        list_jobs();
        return;
    }
    else if (strcmp(command, "fcfs") == 0) {
        set_pending_policy_change(POLICY_FCFS);
        return;
    }
    else if (strcmp(command, "sjf") == 0) {
        set_pending_policy_change(POLICY_SJF);
        return;
    }
    else if (strcmp(command, "priority") == 0) {
        set_pending_policy_change(POLICY_PRIORITY);
        return; 
    }
    else if (strcmp(command, "test") == 0) {
        printf("test\n");
        return;
    }
    else if (strcmp(command, "quit") == 0) {
        stop_scheduler();
        print_performance_metrics();
        exit(0);
    }
    else {
        printf("Unknown command\n"); 
    }
}

typedef struct {
    const char *command;
    const char *description;
} menu_item;

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