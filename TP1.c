#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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

    displayWelcomeMessage();

    displayPrompt();

    return 0;
}

