#include "includes.h"

// Put \0 in place of # because everything after is comment
void deleteComment(char *str)
{
    char *commentPosition = strchr(str, '#');

    if (commentPosition != NULL)
    {
        *commentPosition = '\0';
    }
}

// Split input on specified character
void parseCommand(char buffer[MAX_COMMAND_LEN], char *split_buffer[MAX_COMMAND_LEN], const char *split_on, int *index)
{
    split_buffer[0] = strtok(buffer, split_on);

    while (split_buffer[*index] != NULL)
    {
        *index += 1;
        split_buffer[*index] = strtok(NULL, split_on);
    }
}

void execute(char *command, char *argument_list[], int input_fd, int output_fd)
{
    // Fork the program, because the command executed by execvp will take over the .c program
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Redirecting input and output if < or > present in command
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);

        // Newly spawned child process
        int status_code = execvp(command, argument_list);

        if (status_code == -1)
        {
            write(output_fd, "Incorrect command\n\0", 20);
        }
    }
    else
    {
        // Wait for the forked child process to end
        wait(NULL);
    }
}