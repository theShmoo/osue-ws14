/**
 * @file parent.c
 * @author David Pfahler (1126287) <e1126287@student.tuwien.ac.at>
 * @brief This file contains the implementation of the parent process of the calculator. It waits for user input on the stdin and prints the output from the child process to the stdout 
 * @date 27.04.2014
 */

#include "calculator.h"

/**
*@brief global variable for reading from the pipe from the child process
*/
FILE * reading;

/**
*@brief global variable for writing to the pipe to the child process
*/
FILE * writing;

/**
 * @brief gloabal variable for cleanup
 */
int * pipes_global;

/* STATIC FUNCTIONS */

/**
 * @brief free all resources from the child
 * @details closes pipes
 */
void free_parent_resources( void ){
	DEBUG("Start closing parent process\n");
	(void) fclose(writing);
	(void) fclose(reading);
	(void) close(*(pipes_global) + CHILD_WRITE);
	(void) close(*(pipes_global) + PARENT_READ);

	DEBUG("Wait for the child to close\n");

	if(wait(NULL) == -1){
		(void) fprintf(stderr,"No child process found\n");	
		exit(EXIT_FAILURE);
	}

	DEBUG("parent closed\n");
}

/* IMPLEMENTATIONS */

void bail_out_parent(int eval, const char * fmt, ...){
	DEBUG("bail out parent started\n");
	free_parent_resources();
	va_list arglist;
	va_start(arglist, fmt);
	bail_out(eval,fmt,arglist);
	va_end(arglist);
}

void parentProcess( int* pipes){

	DEBUG("starting parent process\n");

	pipes_global = pipes;

	reading = fdopen(*(pipes + PARENT_READ), "r");
	if (reading == NULL) {
		bail_out_parent(EXIT_FAILURE,"parent failed reading pipe");
	}
	writing = fdopen(*(pipes + CHILD_WRITE), "w");
	if (writing == NULL) {
		bail_out_parent(EXIT_FAILURE,"parent failed writing pipe");
	}

	if(close(*(pipes + CHILD_READ)) != 0) {
		bail_out_parent(EXIT_FAILURE,"close + 1 failed");
	}
	if(close(*(pipes + PARENT_WRITE)) != 0) {
		bail_out_parent(EXIT_FAILURE,"close + 2 failed");
	}

	/* Get Input */
	char input[INPUT_BUFFER_LENGTH + 2];
	char result[RESULT_BUFFER_LENGTH + 1];	

	while(fgets(input,INPUT_BUFFER_LENGTH + 1, stdin) != NULL){
		DEBUG("parent received: %s\n",input);
		if( fprintf(writing, "%s", input)<0){
			bail_out_parent(EXIT_FAILURE,"writing to child via pipe failed");
		}
		if( fflush(writing) != 0){
			bail_out_parent(EXIT_FAILURE,"flushing the pipe to child failed");
		}
		DEBUG("parent sent: %s to child\n",input);

		if( fgets(result, RESULT_BUFFER_LENGTH, reading) != NULL){
			DEBUG("parent received %s from child\n",result);
			(void) fprintf(stdout, "%s", result);
		} else{
			bail_out_parent(EXIT_FAILURE,"the reading of the result from the client got an error");
		}
	}

	if ( feof(stdin) == 0 ){
		bail_out_parent(EXIT_FAILURE,"reading from stdin failed");
	}

	free_parent_resources();
}
