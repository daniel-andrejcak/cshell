#include "includes.h"

char username[MAX_USERNAME_LEN];
char computername[MAX_HOSTNAME_LEN];
int timeout_time = 30;

// Function to print help file when -h switch is present
void printHelpStrning()
{
    static const char *help_strning[] = {
        "Author: Daniel Andrejcak\n",
        "Interactive client-server shell written in C\n",
        "Commands:\n\thalt\t\tends the program (server)\n\tquit\t\tends the connection (client)\n\thelp\t\twrites help file\n\tstat\t\tlists all connections (server)\n\tabort n\t\tends specified connection (server)\n",
        "Switches:\n\t-c\t\tStart program as client\n\t-h\t\tPrint help file\n\t-i [IP_ADDRESS]\tSet ip address\n\t-n [FILE]\tStart non-interactive mode - read commands from FILE\n\t-p [PORT]\tSet port\n\t-s\t\tStart program as server\n\t-t [TIME]\tSet timeout for innactive connections"};

    fprintf(stdout, "\n");
    for (size_t i = 0; i < 4; i++)
    {
        fprintf(stdout, "%s\n", help_strning[i]);
    }
    fprintf(stdout, "\n");
}

int main(int argc, char *argv[])
{
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

    // Default socket address
    char *serverIP = "127.0.0.1";
    int port = 12345;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "chi:n:p:st:")) != -1)
    {
        switch (opt)
        {
        // Switch for inicializing program as client
        case 'c':
            isClient = true;
            break;

        // Switch for printing help file
        case 'h':
            printHelpStrning();
            return 0;

        // Switch for setting ip address - value needed - IP address
        case 'i':
            serverIP = optarg;
            break;

        // Switch for Non-interactive mode, reads commands from file - value needed - path to file
        case 'n':
            char buffer[MAX_COMMAND_LEN];

            int input_fd = open(optarg, O_RDONLY);

            int bytesReceived;
            if ((bytesReceived = read(input_fd, buffer, sizeof(buffer))) > 0)
            {
                buffer[bytesReceived] = '\0'; // Null-terminate the received data

                processCommand(1, buffer);
            }

            return 0;

        // Switch for setting port - value needed - port
        case 'p':
            port = atoi(optarg);
            break;

        // Switch for inicializing program as server
        case 's':
            isClient = false;
            break;

        // Switch for setting timeout for innactive connections - value needed - time after which connecion will be terminated
        case 't':
            timeout_time = atoi(optarg);
            break;

        // If another args are provided
        default:
            fprintf(stderr, "Invalid option. Use -h for help\n");
            return 1;
        }
    }

    // Start program as server or client node
    if (isClient)
    {
        clientNode(serverIP, port);
    }
    else
    {
        serverNode(serverIP, port);
    }

    return 0;
}