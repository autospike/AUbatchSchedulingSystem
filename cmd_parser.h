/**
 * COMP 7500 Project 3 - AUbatch, A Thread-based Batch Scheduling System
 * William Baker
 * Auburn University
 * 
 * Header file for cmd_parser.c
 * 
 * This file containts the declarations necessary for processing commands and printing the hepl menu
 */

#ifndef CMD_PARSER_H
#define CMD_PARSER_H

void cmd_parser(void);
static void process_command(char *line);
static void help_menu();

#endif