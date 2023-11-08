#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>

#define BUFF_SIZE 1024

void sendString(int socket)
{
    char buffer[BUFF_SIZE];

    ssize_t bytes_sent, bytes_recv;


    while(1) {
        //Implement special header for string
        memset(buffer, 0, sizeof(buffer));
        buffer[0] = 'S';
        printf("Enter a string (or press Enter to exit): ");
        fflush(stdin);
        fgets(buffer+1, sizeof(buffer)-1, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0) {
            // Close connection
            printf("Close connection\n");
            bytes_sent = send(socket, buffer, 0, 0);
            if (bytes_sent < 0) {
                perror("String sending error: ");
                exit(1);
            }
            return;
        }

        bytes_sent = send(socket, buffer, strlen(buffer), 0);
        if (bytes_sent < 0) {
            perror("String sending error: ");
            exit(1);
        }

        bytes_recv = recv(socket, buffer, sizeof(buffer), 0);
        if (bytes_sent < 0) {
            perror("String sending error: ");
            exit(1);
        }

        printf("Server: %s\n", buffer);
    }
}

void sendMessage(int socket, const char *message) {
    ssize_t bytes_sent;
    bytes_sent = send(socket, message, strlen(message), 0);
    if (bytes_sent < 0) {
        perror("Send message error ");
        exit(1);
    }
}

void sendFile(int socket) {
    char fileName[BUFF_SIZE];
    char buffer[BUFF_SIZE];

    FILE *file;

    printf("Enter the path to the file: ");
    fgets(fileName, sizeof(fileName), stdin);
    fileName[strcspn(fileName, "\n")] = '\0';

    file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("File open error");
        return;
    }

    memset(buffer, 0, sizeof(buffer));
    
    size_t bytesRead;
    ssize_t bytes_sent, bytes_recv;

    //Special Header
    buffer[0] = 'F';
    strcpy(buffer+1, fileName);

    sendMessage(socket, buffer);
    bytes_recv = recv(socket, buffer, BUFF_SIZE - 1, 0);
    if (bytes_recv < 0) {
        perror("Error: end of send file name");
    } 
    
    int numRead;
    char new_buffer[64] = "";
    do {
        numRead = fread(new_buffer, 1, sizeof(new_buffer), file);
        bytes_sent = send(socket, new_buffer, numRead, 0);
        if (bytes_sent <= 0)
        {
            printf("\nConnection closed!\n");
            break;
        }

        bytes_recv = recv(socket, buffer, BUFF_SIZE - 1, 0);
        if (bytes_recv <= 0)
        {
            printf("\nError!Cannot receive data from sever!\n");
            break;
        }
    } while(numRead == sizeof(new_buffer));

    sendMessage(socket, "end");
    
    printf("\nFile sent successfully!\n");

    fclose(file);
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IpAddress> <PortNumber>\n", argv[0]);
        exit(1);
    }

    int client_sock;
    struct sockaddr_in server_addr;

    // Step 1: Create TCP socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error: ");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        exit(1);
    }

    // Step 2: Connect to server
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection error: ");
        exit(1);
    }

    int choice;
    do {
        printf("\nMENU\n");
        printf("-----------------------------------\n");
        printf("1. Gui xau bat ky\n");
        printf("2. Gui noi dung mot file\n");
        printf("Your choice (1-2, other to quit): ");

        scanf("%d", &choice);
        getchar();

        switch(choice) {
            case 1:
                sendString(client_sock);
                break;

            case 2:
                sendFile(client_sock);
                break;

            default:
                close(client_sock);
                return 0;
        }
    } while(1);

    close(client_sock);
    return 0;
}


