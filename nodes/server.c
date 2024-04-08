#include "../header_files/server.h"
#include "../header_files/includes.h"
#include "server/parse_commands.h"

void *handleClient(void *arg)
{
    int clientSocket = *((int *)arg);
    char buffer[MAX_COMMAND_LEN];

    while (true)
    {
        int bytesReceived = read(clientSocket, buffer, sizeof(buffer));
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data

            if (strncmp(buffer, "halt", 4) == 0)
            {
                printf("Connection terminated\n");
                break;
            }

            deleteComment(buffer);

            int commandCount = 0;
            char *splitBuffer[MAX_COMMAND_LEN];

            // split input on ; to separate different independant commands
            splitCommands(buffer, splitBuffer, ";", &commandCount);

            // redirecting output to socket - default + for error messages
            dup2(clientSocket, STDOUT_FILENO);

            for (size_t i = 0; i < commandCount; i++)
            {
                char *parsedCommand[1024];
                int index = 0;
                parseCommand(splitBuffer[i], parsedCommand, &index);

                int input_fd = STDIN_FILENO;
                int output_fd = clientSocket;

                if (index > 2 && (strcmp(parsedCommand[index - 2], "<") == 0 || strcmp(parsedCommand[index - 2], ">") == 0))
                {
                    if (strcmp(parsedCommand[index - 2], "<") == 0)
                    {
                        input_fd = open(parsedCommand[index - 1], O_RDONLY);
                    }

                    if (strcmp(parsedCommand[index - 2], ">") == 0)
                    {
                        output_fd = open(parsedCommand[index - 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                    }

                    parsedCommand[index - 1] = NULL;
                    parsedCommand[index - 2] = NULL;

                    if (index > 4 && (strcmp(parsedCommand[index - 4], "<") == 0 || strcmp(parsedCommand[index - 4], ">") == 0))
                    {
                        if (strcmp(parsedCommand[index - 4], "<") == 0)
                        {
                            input_fd = open(parsedCommand[index - 3], O_RDONLY);
                        }

                        if (strcmp(parsedCommand[index - 4], ">") == 0)
                        {
                            output_fd = open(parsedCommand[index - 3], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                        }

                        parsedCommand[index - 3] = NULL;
                        parsedCommand[index - 4] = NULL;
                    }
                }

                executeCommand(parsedCommand[0], parsedCommand, input_fd, output_fd);
            }
        }
    }

    // Free up the resources
    close(clientSocket);
    free(arg);
    pthread_exit(NULL);
}

void serverSide(char *address, int port)
{
    int serverSocket;
    struct sockaddr_in serverAddr;

    // set socket name and type - local socket
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, address, &serverAddr.sin_addr) < 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // serverSocket is main socket only listening for incoming connections
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // if socket creation failed, then exit the program
    if (serverSocket < 0)
    {
        perror("Failed to create socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // bind socket with local address
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Failed to bind to a socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // listen for max maout of connections
    if (listen(serverSocket, SOMAXCONN) < 0)
    {
        perror("Listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening for incoming connections...\n");

    while (true)
    {
        // accept one connection
        int newClientSocket = accept(serverSocket, NULL, NULL);
        if (newClientSocket < 0)
        {
            perror("Accept failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }

        printf("Client connected.\n");

        // make thread for each connecting client
        pthread_t tid;
        int *clientSockPtr = (int *)malloc(sizeof(int));
        *clientSockPtr = newClientSocket; // this needs to be dynamically allocated because the newClientSocket can be overwritten while previous thread is still using it
        if (pthread_create(&tid, NULL, handleClient, clientSockPtr) != 0)
        {
            perror("Failed to create thread for client");
            close(newClientSocket);
            free(clientSockPtr);
            continue;
        }

        // Detach the thread to allow it to run independently
        pthread_detach(tid);
    }

    close(serverSocket);
}