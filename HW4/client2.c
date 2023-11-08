#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024
#define CLIENT_PORT 8000
#define CLIENT_ADDR "127.0.0.1"

int main(int argc, char const *argv[])
{

    int client_sock;
    
    int bytes_received_digit, bytes_received_alpha;

    struct sockaddr_in client_addr;
    int sin_size;

    // Step 1: Construct a UDP socket
    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("\nError: construct UDP socket");
		exit(0);
    }

    // Step 2: Bind address to socket
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&client_addr.sin_zero, 8);

    if (bind(client_sock, (struct sockaddr*)&client_addr, sizeof(struct sockaddr)) == -1) {
        perror("\nError: ");
        exit(0);
    }
    
    printf("Listening message from server...\n");

    char alpha_chars[BUFF_SIZE];
    char digit_chars[BUFF_SIZE];

    while(1) {
        memset(alpha_chars, 0, sizeof(alpha_chars));
        memset(digit_chars, 0, sizeof(digit_chars));

        sin_size = sizeof(client_addr);

        bytes_received_alpha = recvfrom(client_sock, alpha_chars, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &sin_size);
        bytes_received_digit = recvfrom(client_sock, digit_chars, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &sin_size);

        if (bytes_received_alpha < 0 || bytes_received_digit < 0) {
            perror("\nError: ");
            close(client_sock);
            return 0;
        } else {
            alpha_chars[bytes_received_alpha] = '\0';
            digit_chars[bytes_received_digit] = '\0';

            printf("%s\n%s\n\n", alpha_chars, digit_chars);
        }
    }

    close(client_sock);
    return 0;
}
