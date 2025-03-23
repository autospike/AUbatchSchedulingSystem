/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * main.c is the "controller" for the AUbatch program.
 * main.c calls start_scheduler, which creates the scheduler and dispatching threads.
 * It also calls cmd_parser, which creates the UI and takes user input
 * 
 * This program was compiled with a make file that should be present in the same directory as this file.
 * 
 */

#include <stdio.h>
#include <ctype.h>
#include "cmd_parser.h"
#include "scheduling.h"

int main(void) {
    start_scheduler();
    cmd_parser();
}
