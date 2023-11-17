#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "accounts.h"
#include "constants.h"

#define MAX_BUFFER_SIZE 1024
#define LISTENQ 1

void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    pid = waitpid(-1, &stat, WNOHANG);
    printf("Child process id %d terminated\n", pid);
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    loadFile();

    int port_number = atoi(argv[1]);

    int server_sock, client_sock, n;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];

    // Create socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    // Bind socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, LISTENQ) == -1) {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port_number);

    signal(SIGCHLD, sig_chld);
    while (1)
    {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0)
        {
            perror("Error in accepting connection");
            exit(EXIT_FAILURE);
        } else {
            printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr));
        }

        client_len = sizeof(client_addr);
        pid_t child_pid = fork();
        if (child_pid < 0)
        {
            perror("Fork error");
            exit(EXIT_FAILURE);
        }
        else if (child_pid == 0)
        {
            // Child process
            n = recv(client_sock, (char *)buffer, MAX_BUFFER_SIZE, 0);
            buffer[n] = '\0';
            char username[MAX_BUFFER_SIZE];
            strcpy(username, buffer);
            printf("Received username: %s\n", username);

            Account *account = searchAccount(head, username);
            if (account == NULL)
            {
                char *ack = "Account not found";
                send(client_sock, ack, strlen(ack), 0);
            }
            // else if (account->status == 0)
            // {
            //     char *ack = "Account is not ready";
            //     send(client_sock, ack, strlen(ack), 0);
            // }
            else
            {
                printf("Found username: %s\n", account->username);
                char *ack = "Insert password";
                send(client_sock, ack, strlen(ack), 0);

                int countLogin = 0;
                int loginSucess = 0;
                while (countLogin < 3)
                {
                    n = recv(client_sock, (char *)buffer, MAX_BUFFER_SIZE, 0);
                    buffer[n] = '\0';
                    char password[MAX_BUFFER_SIZE];
                    strcpy(password, buffer);
                    printf("Received password: %s\n", password);
                    if (strcmp(account->password, password) != 0)
                    {
                        countLogin++;
                        //char *ack = "Not OK";
                        char *ack = NOT_OK;
                        send(client_sock, (const char *)ack, strlen(ack), 0);
                    }
                    else
                    {
                        // Login success
                        loginSucess = 1;
                        break;
                    }
                    if (countLogin >= 3)
                    {
                        account->status = 0;
                        char *ack = "Account is blocked. ";
                        send(client_sock, (const char *)ack, strlen(ack), 0);
                        updateFile();
                        break;
                    }
                }
                // Begin login success
                if (account->status == 1 && loginSucess)
                {
                    loggedInAccount = account;
                    printf("Login as username: %s\n", loggedInAccount->username);

                    //char *ack = "OK";
                    char *ack = OK;
                    send(client_sock, (const char *)ack, strlen(ack), 0);
                    
                    while(1) {
                        //Receiving client message until "bye"
                        n = recv(client_sock, (char *)buffer, MAX_BUFFER_SIZE, 0);
                        buffer[n] = '\0';

                        //print message from client
                        printf("%s: %s\n", loggedInAccount->username, buffer);
                        
                        if (strcmp(buffer, LOG_OUT) == 0)
                        {
                            //Log out, send goodbye
                            char bye[150] = "Goodbye ";
                            strcat(bye, loggedInAccount->username);
                            send(client_sock, (const char *)bye, strlen(bye), 0);

                            loggedInAccount = NULL;
                            break;
                        } else {
                            char *ack = MESSAGE_OK;
                            send (client_sock, (const char *)ack, strlen(ack), 0);
                        }
                    }
                }
                else if (loginSucess && account->status == 0)
                {
                    //Send response to Blocked account
                    char *ack = "Account not ready. ";
                    send(client_sock, (const char *)ack, strlen(ack), 0);
                }
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            //Parent process
            close(client_sock);
        }
    }
    close(server_sock);
    return 0;
}
