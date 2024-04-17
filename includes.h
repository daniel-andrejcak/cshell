#include <arpa/inet.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 1024
#define MAX_PROMPT_LEN 256
#define MAX_USERNAME_LEN 32
#define MAX_HOSTNAME_LEN 64
#define DEFAULT_PORT 8080
// #define LOCAL_SOCKET "./local_socket"

extern char username[MAX_USERNAME_LEN];
extern char computername[MAX_HOSTNAME_LEN];
extern int timeout_time;

void printHelpStrning();

void clientNode(const char *address, int port);

void serverNode(char *address, int port);
void processCommand(int client_socket, char *buffer);

void deleteComment(char *str);
void parseCommand(char buffer[MAX_COMMAND_LEN], char *split_buffer[MAX_COMMAND_LEN], const char *split_on, int *index);
void execute(char *command, char *argument_list[], int input_fd, int output_fd);
