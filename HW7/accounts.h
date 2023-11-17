#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 1024
#define LISTENQ 1

typedef struct _Account {
    char username[255];
    char password[255];
    int status;
    int login_attemps;
    struct _Account *next;
} Account;

Account *newAccount(const char *username, const char *password, const int status)
{
    Account *data = (Account *)malloc(sizeof(Account));
    strcpy(data->username, username);
    strcpy(data->password, password);
    data->status = status;
    data->login_attemps = 0;
    data->next = NULL;

    return data;
}

void printAccount(Account *head)
{
    if (head == NULL)
        return;
    printf("Username=%s, Password=%s, Status=%d\n", head->username, head->password, head->status);
}

void printList(Account *head)
{
    Account *temp = head;
    while (temp != NULL)
    {
        printAccount(temp);
        temp = temp->next;
    }
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
        fprintf(file, "%s %s %d\n", temp->username, temp->password, temp->status);
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
        fprintf(stderr, "Cannot find the file with name account.txt");
        exit(EXIT_FAILURE);
    }

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char username[256], password[256];
        int status;

        int numItems = sscanf(line, "%s %s %d", username, password, &status);

        Account *temp = newAccount(username, password, status);

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

void signIn()
{
    char username[256], password[256];

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    Account *foundAccount = searchAccount(head, username);
    if (foundAccount != NULL)
    {
        if (strcmp(foundAccount->password, password) == 0)
        {
            // Correct password
            if (foundAccount->status == 1)
            {
                // Account is not blocked
                printf("Hello %s.\n", foundAccount->username);
                foundAccount->login_attemps = 0;
                loggedInAccount = foundAccount;
                return;
            }
            else
            {
                // Account is blocked
                printf("Account is not ready\n");
                return;
            }
        }
        else
        {
            // Wrong password
            foundAccount->login_attemps++;
            printf("Password is incorrect\n");

            if (foundAccount->login_attemps >= 3)
            {
                // Exceeded 3 times wrong password
                foundAccount->status = 0;
                printf("Account is blocked\n");
            }
            updateFile();
        }
    }
    else
    {
        printf("Cannot find account\n");
    }
}

void signOut()
{
    char username[255];

    if (loggedInAccount != NULL)
    {
        printf("Username: ");
        scanf("%s", username);

        if (strcmp(loggedInAccount->username, username) == 0)
        {
            printf("Goodbye %s\n", loggedInAccount->username);
            loggedInAccount = NULL;
        }
        else
        {
            printf("Cannot find account\n");
        }
    }
    else
    {
        printf("Account is not sign in\n");
    }
}
