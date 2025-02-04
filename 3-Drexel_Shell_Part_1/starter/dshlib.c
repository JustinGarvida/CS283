#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || clist == NULL)
    {
        printf("DEBUG: Null command line or command list pointer.\n");
        return WARN_NO_CMDS;
    }

    // Clear the command list structure
    printf("DEBUG: Initializing command list structure.\n");
    memset(clist, 0, sizeof(command_list_t));

    // Split the command line by pipes
    char *command = strtok(cmd_line, PIPE_STRING);
    int command_count = 0;

    while (command != NULL)
    {
        printf("DEBUG: Tokenized command: '%s'\n", command);

        // Check if we've exceeded the maximum allowed commands
        if (command_count >= CMD_MAX)
        {
            printf("DEBUG: Exceeded maximum number of commands (%d).\n", CMD_MAX);
            return ERR_TOO_MANY_COMMANDS;
        }

        // Remove leading whitespace from the command
        printf("DEBUG: Removing leading whitespace if any.\n");
        while (*command == SPACE_CHAR)
        {
            printf("DEBUG: Skipping leading space.\n");
            command++;
        }

        // Debug message to confirm the leading space removal
        printf("DEBUG: Command after removing leading whitespace: '%s'\n", command);

        // Check if the entire command is too large to store
        if (strlen(command) >= ARG_MAX)
        {
            printf("DEBUG: Command exceeds maximum argument size (%d).\n", ARG_MAX);
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Copy the entire command into the args field
        printf("DEBUG: Storing command into args field.\n");
        strncpy(clist->commands[command_count].args, command, ARG_MAX);
        clist->commands[command_count].args[ARG_MAX - 1] = '\0';

        // Debug message to show what was stored
        printf("DEBUG: Stored command: '%s'\n", clist->commands[command_count].args);

        // Increment the command count
        command_count++;
        printf("DEBUG: Incremented command count to %d.\n", command_count);

        // Move to the next command
        command = strtok(NULL, PIPE_STRING);
        printf("DEBUG: Moving to next command.\n");
    }

    // Set the number of parsed commands
    clist->num = command_count;

    // Final debug output
    printf("DEBUG: Total commands parsed: %d\n", clist->num);

    return OK;
}
