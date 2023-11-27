#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>

#define MAX_COMMAND_LENGTH 100
#define PROMPT_2 "enseash [%s:%d|%ldms] %%  "
#define WELCOME_MESSAGE "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n"

// A function used to write the very first welcome message using the write() function
void displayWelcomeMessage() {
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
}

// A function used to write the name of the prompt 'in our case enseash %' + the status depends on the first output.
void displayPrompt(int status, bool isSignal, long executionTime) {
    char prompt[MAX_COMMAND_LENGTH];
    char statusStr[10];
    if (isSignal) {
        snprintf(statusStr, sizeof(statusStr), "sign");
    } else {
        snprintf(statusStr, sizeof(statusStr), "exit");
    }
    snprintf(prompt, sizeof(prompt), PROMPT_2, statusStr, status,executionTime);
    write(STDOUT_FILENO, prompt, strlen(prompt));
}

// Custom FUnction to calculate the exact time passed.
long Time(struct timespec start, struct timespec end) {
    return ((end.tv_sec - start.tv_sec) * 1000) + ((end.tv_nsec - start.tv_nsec) / 1000000);
}

int main() {
    long executionTime = 0; // the first display will have 0ms
    char command[MAX_COMMAND_LENGTH];
    int lastStatus = 0;
    bool isSignal = false;

    displayWelcomeMessage();

	
    while (1) { // We stay in the loop until the user run 'exit'
	displayPrompt(lastStatus, isSignal, executionTime);

	
        if (read(STDIN_FILENO, command, sizeof(command)) <= 0) {
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        // Delete the \n at the end of the read, which we added, since it might cause problems while executing the command
        command[strcspn(command, "\n")] = '\0';
	
	// Check if the user want to exit
	
        if (strcmp(command, "exit") == 0) {
            write(STDOUT_FILENO, "Bye bye ...\n", strlen("Bye bye ...\n"));
            break;
        }
	// Check if the user did not run anything, and then instead we run the command date.
	if (strcmp(command, "") == 0) {
            strcpy(command, "date");
        }
        
        struct timespec start, end;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Create a child process to run the commands
        pid_t pid = fork();

        if (pid < 0) {
            perror("Erreur de fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
           
            char *args[MAX_COMMAND_LENGTH];
            char *pcs = strtok(command, " ");
            int i = 0;
            while (pcs != NULL) {
                args[i++] = pcs;
                pcs = strtok(NULL, " ");
            }
            args[i] = NULL;

            execvp(args[0], args);
            exit(EXIT_FAILURE);
        } else {
            // We wait until the end of the execution in the child process
            int status;
            waitpid(pid, &status, 0);
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            executionTime = Time(start, end);
	    // Grab the status Code which depends on em we will set the lastStatus Variable used in displayPrompt()
            if (WIFEXITED(status)) {
                lastStatus = WEXITSTATUS(status);
                isSignal = false;
            } else if (WIFSIGNALED(status)) {
                lastStatus = WTERMSIG(status);
                isSignal = true;
            }
            
            
           
        }
    }

    return 0;
}

