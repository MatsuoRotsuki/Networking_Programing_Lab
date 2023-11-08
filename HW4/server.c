/*UDP Echo Server*/
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <ctype.h>

#define DEFAULT_PORT 5550
#define TARGET_PORT 8000
#define TARGET_ADDR "127.0.0.1"
#define BUFF_SIZE 1024

/**
 * Ham ma hoa string theo SHA1
*/
char *encode_sha1(const char* input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)input, strlen(input), hash);

    char* result = (char*)malloc(2 * SHA_DIGEST_LENGTH + 1);
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(result + (i * 2), "%02x", hash[i]);
    }

    return result;
}

/**
 * Ham tach chu cai va chu so
*/
void classify_characters(const char* input, char** alpha_chars, char** digit_chars) {
    int alpha_count = 0, digit_count = 0;
    int len = strlen(input);

    for (int i = 0; i < len; i++) {
        if (isalpha(input[i])) {
            (*alpha_chars)[alpha_count++] = input[i];
        } else if (isdigit(input[i])) {
            (*digit_chars)[digit_count++] = input[i];
        }
    }

    (*alpha_chars)[alpha_count] = '\0';
    (*digit_chars)[digit_count] = '\0';
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }

    int PORT = atoi(argv[1]);

    int server_sock; // file descriptors

    int bytes_sent, bytes_received;

    struct sockaddr_in server_addr; // server's address information
    struct sockaddr_in client_addr; // client's address information
    int sin_size;

    // Step 1: Construct a UDP socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("\nError: Cannot construct UDP socket");
        exit(0);
    }

    // Step 2: Bind address to socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY puts your IP address automatically
    bzero(&(server_addr.sin_zero), 8); // zero the rest of the structure

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("\nError: ");
        exit(0);
    }

    printf("Server listening on port %d...\n", PORT);

    char buff[BUFF_SIZE];
    char *alpha_chars = (char*)malloc(sizeof(char) * BUFF_SIZE);
    char *digit_chars = (char*)malloc(sizeof(char) * BUFF_SIZE);

    // Step 3: Get address of target client to send result
    struct sockaddr_in tgt_client_addr;
    tgt_client_addr.sin_family = AF_INET;
    tgt_client_addr.sin_port = htons(TARGET_PORT);
    tgt_client_addr.sin_addr.s_addr = inet_addr(TARGET_ADDR);

    // Step 4: Communicate with clients
    while(1) {
        memset(buff, 0, sizeof(buff));
        sin_size = sizeof(struct sockaddr_in);

        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr*)&client_addr, &sin_size);

        if (bytes_received < 0) {
            perror("\nError: ");
            close(server_sock);
            return 0;
        } else {
            buff[bytes_received] = '\0';
            char* sha1_hash = encode_sha1(buff);
            classify_characters(sha1_hash, &alpha_chars, &digit_chars);
            printf("[%s:%d]: %s", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buff);
        }

        //bytes_sent = sendto(server_sock, buff, bytes_received, 0, (struct sockadrr*)&client_addr, &sin_size); //send to client welcome message
        bytes_sent += sendto(server_sock, alpha_chars, strlen(alpha_chars), 0, (struct sockaddr*)&tgt_client_addr, sizeof(tgt_client_addr));
        bytes_sent += sendto(server_sock, digit_chars, strlen(digit_chars), 0, (struct sockaddr*)&tgt_client_addr, sizeof(tgt_client_addr));

        if (bytes_sent < 0)
            perror("\nError: ");
    }

    close(server_sock);
    return 0;
}