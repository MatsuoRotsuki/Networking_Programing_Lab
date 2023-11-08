#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/md5.h>

#define DEFAULT_PORT 5500
#define BUFF_SIZE 1024
#define MAX_CONNECTION 5

char hex_str[MD5_DIGEST_LENGTH * 2 + 1];
char digest[MD5_DIGEST_LENGTH];
char number[BUFF_SIZE];
char alpha[BUFF_SIZE];

void MD5ToHex(const unsigned char *hash, char *hex)
{
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(hex + 2 * i, "%02x", hash[i]);
    }
    hex[MD5_DIGEST_LENGTH * 2] = '\0';
}

void split(char *hex) {
    int numberCount = 0, alphaCount = 0;
    for (int i = 0; i < strlen(hex); i++)
    {
        if (!isalpha(hex[i]))
            number[numberCount++] = hex[i];
        else
            alpha[alphaCount++] = hex[i];
    }
    number[numberCount] = '\0';
    alpha[alphaCount] = '\0';
}

void splitFileName(const char* input, char* fileName, char* extension)
{
    int dotIndex = strcspn(input, ".");
    if (dotIndex == strlen(input)) return;
    strcpy(extension, input+dotIndex+1);
    strncpy(fileName, input, dotIndex);
}

int checkAlphaNum(const char *str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		if (!isalnum(str[i]))
		{
			return 0;
		}
	}
	return 1;
}

void sendMessage(int socket, const char *message) {
    ssize_t bytes_sent;
    bytes_sent = send(socket, message, strlen(message), 0);
    if (bytes_sent < 0) {
        perror("Send message error ");
        exit(1);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
        exit(1);
    }
    
    int PORT = atoi(argv[1]);

    int server_sock, client_sock;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    
    char buffer[BUFF_SIZE];

    socklen_t addr_size = sizeof(client_addr);

    //Step 1: Construct a TCP socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error: ");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //Step 2: Bind address to socket
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding error: ");
        exit(1);
    }

    if (listen(server_sock, MAX_CONNECTION) < 0) {
        perror("Listening error: ");
        exit(1);
    }

    printf("Server is listening on port %s...\n", argv[1]);

    while(1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("Accept error: ");
            exit(1);
        }

        printf("You got a connection from %s\n", inet_ntoa(client_addr.sin_addr) ); /* prints client's IP */

        memset(buffer, 0, sizeof(buffer));

        ssize_t bytes_recv, bytes_sent;
        while(1) {
            bytes_recv = recv(client_sock, buffer, sizeof(buffer)-1, 0);
            if (strcmp(buffer, "exit") == 0) {
                printf("\nClient has exited\n");
                break;
            }
            if (bytes_recv <= 0) {
                printf("\nConnection closed\n");
                break;
            }
            buffer[bytes_recv] = '\0';
            printf("Client: %s\n", buffer);
            if (buffer[0] == 'S') {
                char str[BUFF_SIZE];
                strcpy(str, buffer+1);
                if (checkAlphaNum(str)) {
                    // Encode MD5
                    MD5(str, strlen(str), digest);
                    MD5ToHex(digest, hex_str);
                    split(hex_str);

                    printf("MD5 Hash: %s\n", hex_str);
                    printf("Number: %s\n", number);
                    printf("Alpha: %s\n\n", alpha);

                    sendMessage(client_sock, "String Ok");
                } else {
                    sendMessage(client_sock, "String must be alphanumeric");
                }
            }
            else {
                char fileName[BUFF_SIZE];
                strcpy(fileName, "copy_of_");
                strcat(fileName, buffer+1);
                printf("File name: %s\n", fileName);
                FILE *file = fopen(fileName, "wb");
                sendMessage(client_sock, "Received file name");
                while(1) {
                    bytes_recv = recv(client_sock, buffer, BUFF_SIZE - 1, 0);
                    if (bytes_recv <= 0) {
                        printf("\nConnection closed\n");
                        break;
                    }
                    buffer[bytes_recv] = '\0';
                    if (strcmp(buffer, "end") == 0)
                        break;

                    fwrite(buffer, 1, 64, file);

                    sendMessage(client_sock, "Receive file successfully");
                }
                printf("\nReceive file successfully\n");
                fclose(file);
            }
        }

        close(client_sock);
        printf("Client disconnected.\n");
    }

    close(server_sock);
    return 0;
}

