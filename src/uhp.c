#include <stdio.h>
#include <string.h>

#include "uhp.h"

#define MAX_COMMAND_LENGTH 32

const char IDENTIFIER[] = "Zo\u00e9 v1.0";

void error(char message[])
{
    printf("err %s\n", message);
}

void info()
{
    printf("id %s\n", IDENTIFIER);
    puts("ok");
}

void uhp_loop()
{
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        command[0] = '\0';
        scanf("%31[^\n]", command);

        if (!strcmp(command, "info")) {
            info();
        } else if (!strcmp(command, "exit")) {
            getchar();
            break;
        } else {
            error("Invalid command.");
        }

        getchar();
    }
}
