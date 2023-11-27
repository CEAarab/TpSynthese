#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 100
#define PROMPT "enseash %  "
#define WELCOME_MESSAGE "Bienvenue dans le Shell ENSEA.\nPour quitter, tapez 'exit'.\n"

// A function used to write the very first welcome message using the write() function
void displayWelcomeMessage() {
    write(STDOUT_FILENO, WELCOME_MESSAGE, strlen(WELCOME_MESSAGE));
}

// A function used to write the name of the prompt 'in our case enseash %'
void displayPrompt() {
    write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    displayWelcomeMessage();

	
    while (1) { // We stay in the loop until the user run 'exit'
        displayPrompt();

        if (read(STDIN_FILENO, command, sizeof(command)) <= 0) {
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        // Delete the \n at the end of the read, which we added in line 29, since it might cause problems while executing the command
        command[strcspn(command, "\n")] = '\0';

        

        // Create a child process to run the commands
        pid_t pid = fork();

        if (pid < 0) {
            perror("Erreur de fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // CExecute the code
            execlp(command, command, NULL);

            // Check if we got any errors while running the command, if yes we print a custom error message
            char errorMsg[MAX_COMMAND_LENGTH + 50];
            
            snprintf(errorMsg, sizeof(errorMsg), "Erreur lors de l'exécution de la commande %s\n", command);	// It is not similar to printf , we only stored the output message in C string
            write(STDOUT_FILENO, errorMsg, strlen(errorMsg));
            exit(EXIT_FAILURE);
        } else {
            // We wait until the end of the execution in the child process
            wait(NULL);
        }
    }

    return 0;
}

