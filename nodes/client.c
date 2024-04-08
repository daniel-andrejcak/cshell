#include "../header_files/client.h"
#include "../header_files/includes.h"

int countCommands(const char *str, char ch)
{
    int count = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ch)
        {
            count++;
        }
    }

    return count + 1;
}

void clientSide(char *username, char *computername, const char *address, int port)
{
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0)
    {
        printf("Invalid address");
        exit(EXIT_FAILURE);
    }

    // Setup socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    size_t buffer_size = 0;

    while (true)
    {
        time_t rawtime = time(NULL);
        struct tm *time_info = localtime(&rawtime);

        char *command_buffer = NULL;

        fprintf(stdout, "%02d:%02d %s@%s$ ", time_info->tm_hour, time_info->tm_min, computername, username);

        ssize_t characters_read = getline(&command_buffer, &buffer_size, stdin);

        if (characters_read == -1)
        {
            printf("Error reading input");
            exit(EXIT_FAILURE);
        }

        command_buffer[characters_read - 1] = '\0';

        // int commandCount = countCommands(command_buffer, ';');

        // ends the program
        if (strncmp(command_buffer, "quit", 4) == 0)
        {
            return; // doplnit nejaku funkcionalitu, aby sa neukoncil program, ale iba toto konkretne spojenie
        }
        // help file
        else if (strncmp(command_buffer, "help", 4) == 0)
        {
            printHelpStrning();
        }
        else
        {
            int commandCount = countCommands(command_buffer, ';');
            // Send input to server
            if (write(client_socket, command_buffer, characters_read) < 0)
            {
                perror("Failed to send data to server");
                break;
            }

            if (strncmp(command_buffer, "halt", 4) == 0)
            {
                exit(EXIT_SUCCESS); // doplnit nejaku funkcionalitu, aby sa neukoncil program, ale iba toto konkretne spojenie
            }

            for (size_t i = 0; i < commandCount; i++)
            {
                char buffer[MAX_COMMAND_LEN] = {'\0'};
                read(client_socket, buffer, MAX_COMMAND_LEN);

                printf("%s", buffer);
            }
        }

        free(command_buffer);
    }
}