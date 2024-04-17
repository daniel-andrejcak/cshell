#include "../includes.h"

void clientNode(const char *address, int port)
{
    struct sockaddr_in server_addr;

    // Set socket type, IP address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(port);

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

    char *command_buffer = NULL;
    size_t buffer_size = 0;

    // Set of FDs to monitor by select()
    fd_set read_fds;

    while (true)
    {
        time_t rawtime = time(NULL);
        struct tm *time_info = localtime(&rawtime);

        fprintf(stdout, "%02d:%02d %s@%s$ ", time_info->tm_hour, time_info->tm_min, computername, username);
        fflush(stdout);

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  // Add stdin to the FD set
        FD_SET(client_socket, &read_fds); // Add client socket to the FD set

        // Using select() to monitor stdin and socket for activity
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
                perror("Getline error");
                exit(EXIT_FAILURE);
            }

            command_buffer[characters_read - 1] = '\0'; // Null-terminate the read data

            // End the client
            if (strncmp(command_buffer, "quit", 4) == 0)
            {
                break;
            }
            else if (strncmp(command_buffer, "help", 4) == 0)
            {
                // Print help file
                printHelpStrning();
            }
            else
            {
                // Send input to server
                if (write(client_socket, command_buffer, characters_read) < 0)
                {
                    perror("Write error");
                    break;
                }
            }
        }

        // Read from socket and print the output
        if (FD_ISSET(client_socket, &read_fds))
        {
            char buffer[MAX_COMMAND_LEN] = {'\0'};
            ssize_t bytes_read = read(client_socket, buffer, MAX_COMMAND_LEN);

            if (bytes_read == 0 || strncmp(buffer, "abort", 5) == 0)
            {
                printf("Connection closed by server\n");
                break;
            }

            printf("%s", buffer);
        }
    }

    free(command_buffer);
    close(client_socket);
}