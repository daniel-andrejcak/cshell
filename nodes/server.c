#include "../includes.h"

// Global array holding socket FDs for all active connections
int active_connections[SOMAXCONN];

void newActiveConnection(int client_socket)
{
    for (size_t i = 0; i < SOMAXCONN; i++)
    {
        if (active_connections[i] == 0)
        {
            active_connections[i] = client_socket;
            break;
        }
    }
}

void deleteFromActiveConnection(int client_socket)
{
    for (size_t i = 0; i < SOMAXCONN; i++)
    {
        if (active_connections[i] == client_socket)
        {
            active_connections[i] = 0;
            break;
        }
    }
}

// Sets input and output file descriptors for command executed by execvp() based on presence of '<' and/or '>' characters in command
void setFDs(int *input_fd, int *output_fd, char *parsedCommand[], int index)
{
    if (index > 2 && (strcmp(parsedCommand[index - 2], "<") == 0 || strcmp(parsedCommand[index - 2], ">") == 0))
    {
        if (strcmp(parsedCommand[index - 2], "<") == 0)
        {
            *input_fd = open(parsedCommand[index - 1], O_RDONLY);
        }

        if (strcmp(parsedCommand[index - 2], ">") == 0)
        {
            *output_fd = open(parsedCommand[index - 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
        }

        parsedCommand[index - 1] = NULL;
        parsedCommand[index - 2] = NULL;

        if (index > 4 && (strcmp(parsedCommand[index - 4], "<") == 0 || strcmp(parsedCommand[index - 4], ">") == 0))
        {
            if (strcmp(parsedCommand[index - 4], "<") == 0)
            {
                *input_fd = open(parsedCommand[index - 3], O_RDONLY);
            }

            if (strcmp(parsedCommand[index - 4], ">") == 0)
            {
                *output_fd = open(parsedCommand[index - 3], O_WRONLY | O_TRUNC | O_CREAT, 0644);
            }

            parsedCommand[index - 3] = NULL;
            parsedCommand[index - 4] = NULL;
        }
    }
}

// Process commands special characters and based on their presence set corresponding functionalities and execute command
void processCommand(int client_socket, char *buffer)
{
    if (strncmp(buffer, "quit", 4) == 0)
    {
        // printf("Socket connection %d terminated\n", client_socket);
        deleteFromActiveConnection(client_socket);
        return;
    }

    // Function to delete everything after first '#'
    deleteComment(buffer);

    int commandCount = 0;
    char *splitBuffer[MAX_COMMAND_LEN];

    // Split input on ';' to separate different independant commands
    parseCommand(buffer, splitBuffer, ";", &commandCount);

    for (size_t i = 0; i < commandCount; i++)
    {
        char *parsedCommand[1024];
        int index = 0;

        // Split command on '<space>' to separate command and all its arguments into array
        parseCommand(splitBuffer[i], parsedCommand, " ", &index);

        // Set default input and output FDs
        int input_fd = STDIN_FILENO;
        int output_fd = client_socket;

        // Set command specific FDs
        setFDs(&input_fd, &output_fd, parsedCommand, index);

        // Execute parsed command
        execute(parsedCommand[0], parsedCommand, input_fd, output_fd);
    }
}

// Function to handle each client connection - for each connection, new thread is made that runs this function
void *handleClient(void *arg)
{
    int client_socket = *((int *)arg);
    fd_set read_fds;

    // Set timeout time if connection became innactive
    struct timeval timeout;
    timeout.tv_sec = timeout_time;
    timeout.tv_usec = 0;

    while (true)
    {
        char buffer[MAX_COMMAND_LEN];

        FD_ZERO(&read_fds);
        FD_SET(client_socket, &read_fds);

        // Monitor socket FD for activity
        int activity = select(client_socket + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("select");
            break;
        }
        else if (activity == 0)
        {
            // No activity within the timeout period, terminate the connection
            // printf("No activity within the timeout period - %d seconds. Terminating connection.\n", timeout_time);

            break;
        }

        // Socket is ready to be read
        if (FD_ISSET(client_socket, &read_fds))
        {
            int bytesReceived = read(client_socket, buffer, sizeof(buffer));
            if (bytesReceived > 0)
            {
                buffer[bytesReceived - 1] = '\0'; // Null-terminate the received data

                // Process the read input
                processCommand(client_socket, buffer);
            }
            else
            {
                // printf("Connection closed.\n");
                break;
            }
        }
    }

    // Connection was terminated
    deleteFromActiveConnection(client_socket);
    close(client_socket);

    return 0;
}

// Accept incoming client connections - function runs in a separate thread and for every new client connection makes separate thread to handle that connection
void *clientAccept(void *arg)
{
    int server_socket = *((int *)(arg));

    while (true)
    {
        // Accept one connection
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0)
        {
            perror("Accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // printf("Client connected.\n");

        // Add to list of active connections
        newActiveConnection(client_socket);

        // Make thread for each connecting client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handleClient, (void *)&client_socket) != 0)
        {
            perror("Failed to create thread for client");

            deleteFromActiveConnection(client_socket);
            close(client_socket);

            continue;
        }

        // Detach the thread to allow it to run independently
        pthread_detach(tid);
    }
}

// Main function when program runs as server
void serverNode(char *address, int port)
{
    struct sockaddr_in server_addr;

    // Set socket type, IP address and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, address, &server_addr.sin_addr) < 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_port = htons(port);

    // server_socket is main socket only listening for incoming connections
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        // Socket creation failed, then exit the program
        perror("Failed to create socket");
        exit(1);
    }

    // Set socket options - reuse IP address and port - no waiting time when socket wasnt closed properly
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket with local address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Failed to bind to a socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incomming connections
    if (listen(server_socket, SOMAXCONN) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Create thread for accepting connections from clients
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, NULL, clientAccept, &server_socket) != 0)
    {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    char *command_buffer = NULL;
    size_t buffer_size = 0;

    // Server side shell functionality
    while (true)
    {
        time_t rawtime = time(NULL);
        struct tm *time_info = localtime(&rawtime);

        fprintf(stdout, "%02d:%02d %s@%s$ ", time_info->tm_hour, time_info->tm_min, computername, username);

        ssize_t characters_read = getline(&command_buffer, &buffer_size, stdin);
        if (characters_read == -1)
        {
            perror("Getline error");
            exit(EXIT_FAILURE);
        }

        command_buffer[characters_read - 1] = '\0'; // Null-terminate the received data

        // End the server - close all client connections
        if (strncmp(command_buffer, "halt", 4) == 0)
        {
            for (size_t i = 0; i < SOMAXCONN; i++)
            {
                if (active_connections[i] == 0)
                {
                    break;
                }

                write(active_connections[i], "abort\0", 6);
                deleteFromActiveConnection(active_connections[i]);

                close(active_connections[i]);
            }

            break;
        }

        // stat command - write all active connections
        else if (strncmp(command_buffer, "stat", 4) == 0)
        {
            fprintf(stdout, "Connections:\n");

            // Iterate through list of active connections, write all client socket FDs
            for (size_t i = 0; i < SOMAXCONN; i++)
            {
                if (active_connections[i] != 0)
                {
                    fprintf(stdout, "%d\n", active_connections[i]);
                }
            }

            continue;
        }

        // abort n command - close active connection, n - client socket FD (from stat)
        else if (strncmp(command_buffer, "abort", 5) == 0 && command_buffer[6] >= 49)
        {
            int socket_to_close = atoi(command_buffer + 6);

            deleteFromActiveConnection(socket_to_close);
            write(socket_to_close, "abort\0", 6);
            close(socket_to_close);
        }
        else
        {
            processCommand(STDOUT_FILENO, command_buffer);
        }
    }

    free(command_buffer);
    close(server_socket);

    exit(EXIT_SUCCESS);
}