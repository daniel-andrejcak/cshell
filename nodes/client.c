#include "../header_files/client.h"
#include "../header_files/includes.h"

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
    fd_set read_fds;

    while (true)
    {
        time_t rawtime = time(NULL);
        struct tm *time_info = localtime(&rawtime);

        char *command_buffer = NULL;

        fprintf(stdout, "%02d:%02d %s@%s$ ", time_info->tm_hour, time_info->tm_min, computername, username);
        fflush(stdout);

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  // Add stdin to the set
        FD_SET(client_socket, &read_fds); // Add client socket to the set

        // using select() monitor stdin and socket for activity
        if (select(client_socket + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Read from stdin and send to server
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            ssize_t characters_read = getline(&command_buffer, &buffer_size, stdin);

            if (characters_read == -1)
            {
                printf("Error reading input");
                exit(EXIT_FAILURE);
            }

            command_buffer[characters_read - 1] = '\0';

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
                // Send input to server
                if (write(client_socket, command_buffer, characters_read) < 0)
                {
                    perror("Failed to send data to server");
                    break;
                }

                if (strncmp(command_buffer, "halt", 4) == 0)
                {
                    close(client_socket);
                    exit(EXIT_SUCCESS); // doplnit nejaku funkcionalitu, aby sa neukoncil program, ale iba toto konkretne spojenie
                }
            }
        }

        if (FD_ISSET(client_socket, &read_fds))
        {
            char buffer[MAX_COMMAND_LEN] = {'\0'};
            read(client_socket, buffer, MAX_COMMAND_LEN);

            printf("%s", buffer);
        }
    }

    close(client_socket);
}