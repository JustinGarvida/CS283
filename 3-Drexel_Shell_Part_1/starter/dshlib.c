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
 */

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || clist == NULL)
    {
        return WARN_NO_CMDS;
    }

    memset(clist, 0, sizeof(command_list_t));

    // Split the command line by pipes
    char *command = strtok(cmd_line, PIPE_STRING);
    int command_count = 0;

    if (command == NULL)
    {
        return WARN_NO_CMDS;
    }

    while (command != NULL)
    {
        // Check if we've exceeded the maximum allowed commands
        if (command_count >= CMD_MAX)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            return ERR_TOO_MANY_COMMANDS;
        }

        // Remove leading whitespace from the command
        while (*command == SPACE_CHAR)
        {
            command++;
        }

        // Check if the command length is too long
        if (strlen(command) >= ARG_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Find the first space in the command to separate executable and arguments
        char *space_pos = strchr(command, SPACE_CHAR);
        if (space_pos != NULL)
        {
            // We found a space; separate executable and arguments
            size_t exe_len = space_pos - command;
            if (exe_len >= EXE_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            // Copy the executable name
            strncpy(clist->commands[command_count].exe, command, exe_len);
            clist->commands[command_count].exe[exe_len] = '\0';

            // Skip the space and copy the remaining string as arguments
            space_pos++;
            while (*space_pos == SPACE_CHAR) // Remove any additional leading spaces from arguments
            {
                space_pos++;
            }

            strncpy(clist->commands[command_count].args, space_pos, ARG_MAX);
            clist->commands[command_count].args[ARG_MAX - 1] = '\0';
        }
        else
        {
            // No space found; the entire command is the executable name
            if (strlen(command) >= EXE_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            strncpy(clist->commands[command_count].exe, command, EXE_MAX);
            clist->commands[command_count].exe[EXE_MAX - 1] = '\0';
        }

        // Increment the command count
        command_count++;

        // Move to the next command
        command = strtok(NULL, PIPE_STRING);
    }

    // Set the number of parsed commands
    clist->num = command_count;

    return OK;
}
