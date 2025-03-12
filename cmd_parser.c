#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "cmd_parser.h"

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
    
}

static void process_command(char *line) {
    line[strcspn(line, "\n")] = '\0';

    char command[16];

    if (sscanf(line, "%15s", command) != 1) {
        return;
    }

    if (strcmp(command, "help") == 0) {
        help_menu();
    }
    else if (strcmp(command, "run") == 0) {
        printf("run\n");
    }
    else if (strcmp(command, "list") == 0) {
        printf("list\n");  
    }
    else if (strcmp(command, "fcfs") == 0) {
        printf("fcfs\n"); 
    }
    else if (strcmp(command, "sjf") == 0) {
        printf("sjf\n"); 
    }
    else if (strcmp(command, "priority") == 0) {
        printf("priority\n"); 
    }
    else if (strcmp(command, "test") == 0) {
        printf("test\n"); 
    }
    else if (strcmp(command, "quit") == 0) {
        printf("quit\n");
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