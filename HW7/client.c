#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include "constants.h"

#define MAX_BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf("Usage: %s <IPAdress> <PortNumber>", argv[0]);
        exit(1);
    }

    char *IPAddr = argv[1];
    int port_number = atoi(argv[2]);

    int sockfd;
    socklen_t n;
    struct sockaddr_in server_addr;

    char username[MAX_BUFFER_SIZE];
    char password[MAX_BUFFER_SIZE];
    char buffer[MAX_BUFFER_SIZE];

    // Create a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IPAddr);
    server_addr.sin_port = htons(port_number);

    // Establish connection to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(EXIT_FAILURE);
    }

    printf("Enter your username: ");
    fgets(username, sizeof(username), stdin);

    username[strcspn(username, "\n")] = '\0';
    send(sockfd, (const char *)username, strlen(username), 0);
    n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);

    buffer[n] = '\0';
    printf("Server response: %s\n", buffer);
    while (strcmp(buffer, "Account not found") == 0)
    {
        char buffer2[MAX_BUFFER_SIZE];
        printf("Enter your username: ");
        fgets(username, sizeof(username), stdin);

        username[strcspn(username, "\n")] = '\0';
        send(sockfd, username, strlen(username), 0);

        n = recv(sockfd, buffer2, MAX_BUFFER_SIZE, 0);
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer2);
        strcpy(buffer, buffer2);
    }

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);

    // Remove the newline character at the end of the password
    password[strcspn(password, "\n")] = 0;
    send(sockfd, password, strlen(password), 0);

    n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
    buffer[n] = '\0';
    printf("Server response: %s\n", buffer);

    while (strcmp(buffer, NOT_OK) == 0)
    {
        printf("Enter your password: ");
        fgets(password, sizeof(password), stdin);

        password[strcspn(password, "\n")] = 0;
        send(sockfd, password, strlen(password), 0);

        n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
        buffer[n] = '\0';
        printf("Server response: %s\n", buffer);
    }
    if (strcmp(buffer, OK) == 0)
    {
        printf("Log in successfully! Send anything to server. Enter \"bye\" to log out.\n");
        while (1)
        {
            char cmd[MAX_BUFFER_SIZE];
            fgets(cmd, sizeof(cmd), stdin);
            cmd[strcspn(cmd, "\n")] = 0;
            if (strlen(cmd) == 0)
            {
                printf("Closing client...\n");
                close(sockfd);
                return 0;
            }
            send(sockfd, cmd, strlen(cmd), 0);
            n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0);
            buffer[n] = '\0';
            printf("Server response: %s\n", buffer);
            if (strcmp(cmd, LOG_OUT) == 0)
            {
                break;
            }
        }
    }
    printf("Closing client...\n");
    close(sockfd);
    return 0;
}
