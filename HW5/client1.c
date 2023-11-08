#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IPAdress> <PortNumber>\n", argv[0]);
        return 0;
    }

    int client_sock;
    int bytes_sent, bytes_received;

    struct sockaddr_in server_addr;
    int sin_size;

    // Step 1: Construct a UDP socket
    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("\nError: construct UDP socket");
		exit(0);
    }

    //Step 2: Define the address of the server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    char buff[BUFF_SIZE];

    int hasLoggedIn = 0;

    //Step 3: Communicate with server
    while(1) {
        if (!hasLoggedIn) {
            char username[256], password[256];
            printf("Type your username: ");
            scanf("%s",username);
            printf("Type your password: ");
            scanf("%s",password);

            sin_size = sizeof(server_addr);

            snprintf(buff, sizeof(buff), "%s %s", username, password);

            bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr*)&server_addr, sin_size);
            if (bytes_sent < 0) {
                perror("Error: ");
                close(client_sock);
                return 0;
            }
        }
    }

    return 0;
}
