#include "../../header_files/includes.h"

#ifndef PARSE_COMMANDS_H
#define PARSE_COMMANDS_H

void deleteComment(char *str);
void splitCommands(char buffer[MAX_COMMAND_LEN], char *splitBuffer[MAX_COMMAND_LEN], const char *splitOnChar, int *index);
void parseCommand(char buffer[MAX_COMMAND_LEN], char *parsedBuffer[MAX_COMMAND_LEN], int *index);
void executeCommand(char *command, char *argument_list[], int input_fd, int output_fd);

#endif