#ifndef INCLUDES_H
#define INCLUDES_H

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
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_COMMAND_LEN 1024
#define MAX_PROMPT_LEN 256
#define MAX_USERNAME_LEN 32
#define MAX_HOSTNAME_LEN 64
#define DEFAULT_PORT 8080
// #define LOCAL_SOCKET "./local_socket"

void printHelpStrning();

#endif // INCLUDES_H