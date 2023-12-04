#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
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
void pipe_func(char* command) {
    char* args[MAX_COMMAND_LENGTH];
    char* token = strtok(command, "|"); // In order to get the first and seconde command
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, "|");
    }
    args[i] = NULL;

    int pipe_fd[2]; 
    if (pipe(pipe_fd) < 0) { // Creation of the pipes
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork(); //first fork to create the first child
    if (pid1 == 0) {        
	close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO); // Redirects the standard output of the first command to the writing end of the pipe
	close(pipe_fd[1]);

        char *pcs = strtok(args[0], " ");
        int j = 0;
        char *command_args[MAX_COMMAND_LENGTH];
        while (pcs != NULL) {
            command_args[j++] = pcs;
            pcs = strtok(NULL, " ");
        }
        command_args[j] = NULL;
        execvp(command_args[0], command_args); //Execute the first command
        exit(EXIT_FAILURE);
    } else {
        pid_t pid2 = fork(); // Second fork to create the 2nd child
        if (pid2 == 0) {
            close(pipe_fd[1]); 
            dup2(pipe_fd[0], STDIN_FILENO); // Redirects the standard output of the second command to the reading end of the pipe
            close(pipe_fd[0]); 


            char *pcs = strtok(args[1], " ");
            int j = 0;
            char *command_args[MAX_COMMAND_LENGTH];
            while (pcs != NULL) {
                command_args[j++] = pcs;
                pcs = strtok(NULL, " ");
            }
            command_args[j] = NULL;

            execvp(command_args[0], command_args); // Normal final execution like in the main
            exit(EXIT_FAILURE);
        } else {
        //closes all in the parent process
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}
// FUnction to check if there are any redirection symbols, and execute the commands depends on the situation.
void redirections_func(char* command) {
    char* input_file = NULL;
    char* output_file = NULL;

    char* token = strtok(command, " ");
    int i = 0;
    char* args[MAX_COMMAND_LENGTH];

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            input_file = token; // the next token will be considered as input file
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            output_file = token; // the next token will be considered as output file
        } else {
            args[i++] = token; // Collect the arguments + command
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
	
	
    if (input_file != NULL) {
        int in = open(input_file, O_RDONLY);
        dup2(in, STDIN_FILENO);
        close(in);
    }
    if (output_file != NULL) {
        int out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(out, STDOUT_FILENO);
        close(out);
    }

    execvp(args[0], args);	// EXecute the first argument (command) + other args (Arguments)
    exit(EXIT_FAILURE);
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
           
            if(strstr(command, "<") != NULL || strstr(command, ">") != NULL){
            
		    redirections_func(command); // if there are any redirections in the command call the redirections_func() function
		    
            } else if(strstr(command, "|") != NULL){
            
            	    pipe_func(command);
            
            }else {
            	    char *args[MAX_COMMAND_LENGTH]; 
		    char *pcs = strtok(command, " "); //Split the command into tokens, in order to get the arguments and the command
		    int i = 0;
		    while (pcs != NULL) {
		        args[i++] = pcs;
		        pcs = strtok(NULL, " ");
		    }
		    args[i] = NULL;

		    execvp(args[0], args);
		    exit(EXIT_FAILURE);
            }
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

