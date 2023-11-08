#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define DEFAULT_PORT 5500
#define BUFF_SIZE 1024

typedef struct _Account
{
    char username[256];
    char password[256];
    int status;
    int wrongPasswordCount;
    struct _Account *next;
} Account;

Account *newAccount(const char *username, const char *password, const int status, const int wrongPasswordCount)
{
    Account *data = (Account *)malloc(sizeof(Account));
    strcpy(data->username, username);
    strcpy(data->password, password);
    data->status = status;
    data->wrongPasswordCount = wrongPasswordCount;
    data->next = NULL;

    return data;
}

Account *append(Account *head, Account *account)
{
    if (head == NULL)
    {
        head = account;
        return head;
    }
    else
    {
        head->next = append(head->next, account);
        return head;
    }
}

FILE *file;
Account *head = NULL;
Account *loggedInAccount = NULL;

void updateFile()
{
    file = fopen("account.txt", "r+");

    Account *temp = head;
    while (temp != NULL)
    {
        fprintf(file, "%s %s %d %d\n", temp->username, temp->password, temp->status, temp->wrongPasswordCount);
        temp = temp->next;
    }

    fclose(file);
}

void freeList(Account *h)
{
    while (h != NULL)
    {
        Account *temp = h;
        h = h->next;
        free(temp);
    }
}

void loadFile()
{
    file = fopen("account.txt", "r+");

    if (file == NULL)
    {
        exit(EXIT_FAILURE);
    }

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char username[256], password[256];
        int status, wrongPasswordCount;

        int numItems = sscanf(line, "%s %s %d %d", username, password, &status, &wrongPasswordCount);

        if (numItems < 4)
        {
            wrongPasswordCount = 0;
        }

        Account *temp = newAccount(username, password, status, wrongPasswordCount);

        head = append(head, temp);
    }

    fclose(file);
}

Account *searchAccount(Account *head, const char *username)
{
    Account *current = head;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
            return current;
        current = current->next;
    }

    return NULL;
}

//Login

//Logout

//Change Password

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
        return 1;
    }

    int PORT = atoi(argv[1]);

    int server_sock;

    int bytes_sent, bytes_received;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int sin_size;

    //Step 1: Construct a UDP socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("\nError: Cannot construct UDP socket");
        exit(0);
    }

    //Step 2: Bind address to socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("\nError");
        exit(0);
    }

    printf("Server listening on port %d...\n", PORT);

    // Initialization
    char buff[BUFF_SIZE];

    loadFile();

    //Step 3: Communicate with clients
    while(1) {
        memset(buff, 0, sizeof(buff));
        sin_size = sizeof(struct sockaddr_in);

        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &sin_size);
        
        if (bytes_received < 0) {
            perror("\nError");
            close(server_sock);
            return 0;
        } else {
            char str1[255], str2[255];
            if (sscanf(buff, "%s %s", str1, str2) == 2) {
                Account *foundAccount = searchAccount(head, str1);
                if (foundAccount != NULL) {
                    if (strcmp(foundAccount->password, str2) == 0)
                    {
                        //Corect password
                        if (foundAccount->status = 1)
                        {
                            //Logged in
                            foundAccount->wrongPasswordCount = 0;
                            loggedInAccount = foundAccount;
                        } else {
                            printf("Account is not ready\n");
                        }
                    } else {
                        //Wrong password
                        foundAccount->wrongPasswordCount++;
                        printf("Not OK\n");

                        if (foundAccount->wrongPasswordCount >= 3)
                        {
                            foundAccount->status = 0;
                            printf("Account is block\n");
                        }
                        updateFile();
                    }
                } else {
                    printf("Cannot find account");
                }
            }
            else if (sscanf(buff, "%s", str1) == 1) {
                
            }
        }
    }

    close(server_sock);
    return 0;
}
