#include "header_files/includes.h"
#include "header_files/client.h"
#include "header_files/server.h"

void printHelpStrning()
{
    static const char *help_strning[] = {
        "Author: Daniel Andrejcak",
        "Commands:\n\thalt - ends the program\n\tquit - ends the connection\n\thelp - writes help file"};

    fprintf(stdout, "\n");
    for (size_t i = 0; i < 2; i++)
    {
        fprintf(stdout, "%s\n", help_strning[i]);
    }
    fprintf(stdout, "\n");
}

int main(int argc, char *argv[])
{
    char username[MAX_USERNAME_LEN];
    char computername[MAX_HOSTNAME_LEN];

    // Get the name of the user
    struct passwd *pwd = getpwuid(getuid());
    if (!pwd)
    {
        perror("Failed to get username");
        return EXIT_FAILURE;
    }
    strcpy(username, pwd->pw_name);

    // Get the name of the computer
    if (gethostname(computername, MAX_HOSTNAME_LEN) != 0)
    {
        perror("Failed to get computername");
        return EXIT_FAILURE;
    }

    int opt = 0;
    bool isClient = false;

    // default socket address
    char *serverIP = "127.0.0.1";
    int port = 12345;

    // Parse command-line arguments

    while ((opt = getopt(argc, argv, "hp:i:cs")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printHelpStrning();
            return 0;
        case 'i':
            serverIP = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 's':
            isClient = false;
            break;
        case 'c':
            isClient = true;
            break;
        default:
            fprintf(stderr, "Invalid option. Use -h for help\n");
            return 1;
        }
    }

    if (isClient)
    {
        clientSide(username, computername, serverIP, port);
    }
    else
    {
        serverSide(serverIP, port);
    }

    return 0;
}