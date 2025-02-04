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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || clist == NULL)
    {
        return WARN_NO_CMDS;
    }

    // Clear the command list structure
    memset(clist, 0, sizeof(command_list_t));

    // Split the command line by pipes
    char *command = strtok(cmd_line, PIPE_STRING);
    int command_count = 0;

    while (command != NULL)
    {
        // Debug output
        printf("DEBUG: strtok produced command segment: '%s'\n", command);

        // Check if we've exceeded the maximum allowed commands
        if (command_count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Remove leading and trailing whitespace from the command
        while (*command == SPACE_CHAR)
            command++; // Trim leading whitespace

        // Trim trailing whitespace
        char *end = command + strlen(command) - 1;
        while (end > command && isspace(*end))
        {
            *end = '\0';
            end--;
        }

        // Parse the executable name (first token)
        char *exe_name = strtok(command, " ");
        if (exe_name == NULL || strlen(exe_name) >= EXE_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Copy the executable name to the command structure
        strncpy(clist->commands[command_count].exe, exe_name, EXE_MAX);
        clist->commands[command_count].exe[EXE_MAX - 1] = '\0';

        // Parse the arguments (remaining part of the command)
        char *args = strtok(NULL, "");
        if (args != NULL)
        {
            // Remove leading whitespace in arguments
            while (*args == SPACE_CHAR)
                args++;

            if (strlen(args) >= ARG_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            strncpy(clist->commands[command_count].args, args, ARG_MAX);
            clist->commands[command_count].args[ARG_MAX - 1] = '\0';
        }

        // Increment the command count
        command_count++;

        // Move to the next command
        command = strtok(NULL, PIPE_STRING);
    }

    // Set the number of parsed commands
    clist->num = command_count;

    // Debug output
    printf("DEBUG: Total commands parsed: %d\n", clist->num);

    return OK;
}
