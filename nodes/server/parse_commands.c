#include "parse_commands.h"

// puts \0 in place of # because everything after is comment
void deleteComment(char *str)
{
    char *commentPosition = strchr(str, '#');

    if (commentPosition != NULL)
    {
        *commentPosition = '\0';
    }
}

// splits input on ; and threats it as separate commands
void splitCommands(char buffer[MAX_COMMAND_LEN], char *splitBuffer[MAX_COMMAND_LEN], const char *splitOnChar, int *index)
{
    splitBuffer[0] = strtok(buffer, splitOnChar);

    while (splitBuffer[*index] != NULL)
    {
        *index += 1;
        splitBuffer[*index] = strtok(NULL, splitOnChar);
    }
}

// splits command on <space>
void parseCommand(char buffer[MAX_COMMAND_LEN], char *parsedBuffer[MAX_COMMAND_LEN], int *index)
{
    *index = 0;
    parsedBuffer[0] = strtok(buffer, " ");

    while (parsedBuffer[*index] != NULL)
    {
        *index += 1;
        parsedBuffer[*index] = strtok(NULL, " ");
    }
}

void executeCommand(char *command, char *argument_list[], int input_fd, int output_fd)
{

    // forks the program, because the command executed by execvp will take over the .c program
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // redirecting input and output if < or > present in command
        dup2(input_fd, STDIN_FILENO);
        dup2(output_fd, STDOUT_FILENO);

        // newly spawned child process
        int status_code = execvp(command, argument_list);

        if (status_code == -1)
        {
            write(output_fd, "Incorrect command\n\0", 20);
        }
    }
    else
    {
        wait(NULL);
    }
}