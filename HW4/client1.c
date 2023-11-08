/*UDP Echo Server*/
#include <stdio.h>          /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUFF_SIZE 1024

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IPAdress> <PortNumber>\n", argv[0]);
        return 0;
    }

    int client_sock; // file descriptors
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
    char alpha_chars[BUFF_SIZE];
    char digit_chars[BUFF_SIZE];

    //Step 3: Communicate with servers
    while(1) {
        printf("Insert string to send: ");
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);

        char temp[BUFF_SIZE];
        strcpy(temp, buff);
        temp[strcspn(temp, "\n")] = '\0';
        if (strcmp(temp, "@") == 0 || strcmp(temp, "#") == 0)
        {
            close(client_sock);
            printf("Stopping client...");
            return 0;
        }

        sin_size = sizeof(server_addr);

        bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr*)&server_addr, sin_size);
        if (bytes_sent < 0) {
            perror("Error client: ");
            close(client_sock);
            return 0;
        }
        // memset(alpha_chars, 0, sizeof(alpha_chars));
        // memset(digit_chars, 0, sizeof(digit_chars));

        // bytes_received = recvfrom(client_sock, alpha_chars, sizeof(alpha_chars), 0, (struct sockaddr*)&server_addr, &sin_size);
        // bytes_received += recvfrom(client_sock, digit_chars, sizeof(digit_chars), 0, (struct sockaddr*)&server_addr, &sin_size);
        // if (bytes_received < 0) {
        //     perror("Error: ");
        //     close(client_sock);
        //     return 0;
        // }
        // buff[bytes_received] = '\0';
    }
    
    close(client_sock);
    return 0;
}