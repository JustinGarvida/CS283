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

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (cmd_line == NULL || clist == NULL){
        return WARN_NO_CMDS;
    }
    memset(clist, 0, sizeof(command_list_t));
    char *command = strtok(cmd_line, PIPE_STRING);
    int command_count = 0;

    if (command == NULL) {
        return WARN_NO_CMDS;
    }
    while (command != NULL){
        if (command_count >= CMD_MAX){
            return ERR_TOO_MANY_COMMANDS;
        }
        while (*command == SPACE_CHAR) {
            command++;
        }
        if (strlen(command) >= ARG_MAX) {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }
        char *space_pos = strchr(command, SPACE_CHAR);
        if (space_pos != NULL) {
            size_t exe_len = space_pos - command;
            if (exe_len >= EXE_MAX) {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strncpy(clist->commands[command_count].exe, command, exe_len);
            clist->commands[command_count].exe[exe_len] = '\0';
            space_pos++;
            while (*space_pos == SPACE_CHAR) {
                space_pos++;
            }
            strncpy(clist->commands[command_count].args, space_pos, ARG_MAX);
            clist->commands[command_count].args[ARG_MAX - 1] = '\0';
        }
        else {
            if (strlen(command) >= EXE_MAX)  {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            strncpy(clist->commands[command_count].exe, command, EXE_MAX);
            clist->commands[command_count].exe[EXE_MAX - 1] = '\0';
        }
        command_count++;
        command = strtok(NULL, PIPE_STRING);
    }
    clist->num = command_count;
    return OK;
}
