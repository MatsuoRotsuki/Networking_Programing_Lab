#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void registerAccount()
{
    char username[256], password[256];

    printf("Username: ");
    scanf("%s", username);

    // Check if the username has already existed
    Account *alreadyExistAccount = searchAccount(head, username);
    if (alreadyExistAccount != NULL)
    {
        printf("Account existed\n");
        return;
    }

    printf("Password: ");
    scanf("%s", password);

    // Add new account to the list
    Account *newAcc = newAccount(username, password, 1, 0);
    head = append(head, newAcc);
    updateFile();

    printf("Successful registration\n");
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
                foundAccount->wrongPasswordCount = 0;
                loggedInAccount = foundAccount;
                return;
            }
            else
            {
                // Account is blocked
                printf("Account is blocked\n");
                return;
            }
        }
        else
        {
            // Wrong password
            foundAccount->wrongPasswordCount++;
            printf("Password is incorrect\n");

            if (foundAccount->wrongPasswordCount >= 3)
            {
                // Exceeded 3 times wrong password
                foundAccount->status = 0;
                printf("Password is incorrect. Account is blocked\n");
            }
            updateFile();
        }
    }
    else
    {
        printf("Cannot find account\n");
    }
}

void searchForAccount()
{
    if (loggedInAccount == NULL)
    {
        printf("You must login to do this.\n");
        return;
    }
    char username[256];

    printf("Username: ");
    scanf("%s", username);

    Account *foundAccount = searchAccount(head, username);
    if (foundAccount != NULL)
    {
        if (foundAccount->status == 1)
        {
            printf("Account is active\n");
        }
        else
        {
            printf("Account is blocked\n");
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

int main()
{

    loadFile();

    while (1)
    {
        printf("\n\nUSER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Search\n");
        printf("4. Sign out\n");
        printf("Your choice (1-4, other to quit): ");

        int cmd;
        scanf("%d", &cmd);

        switch (cmd)
        {
        case 1:
            registerAccount();
            break;

        case 2:
            signIn();
            break;

        case 3:
            searchForAccount();
            break;

        case 4:
            signOut();
            break;

        default:
            freeList(head);
            return 0;
        }
    }

    return 0;
}
