#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dshlib.h"

/*
 * Implement your main function by building a loop that prompts the
 * user for input. Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input. Since we want fgets to also handle
 * end-of-file (EOF), check the return code of fgets. Example:
 *
 *      while (1) {
 *          printf("%s", SH_PROMPT);
 *          if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
 *              printf("\n");
 *              break;
 *          }
 *          cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
 *          // Implement the rest of the requirements
 *      }
 *
 * Use constants from dshlib.h:
 *      SH_CMD_MAX              Maximum buffer size for user input
 *      EXIT_CMD                Constant that terminates the dsh program
 *      SH_PROMPT               The shell prompt
 *      OK                      Command parsed successfully
 *      WARN_NO_CMDS            User command was empty
 *      ERR_TOO_MANY_COMMANDS   Too many pipes used
 */

int main()
{
    char *cmd_buff = malloc(SH_CMD_MAX);
    if (cmd_buff == NULL)
    {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return EXIT_FAILURE;
    }

    command_list_t clist;

    while (1)
    {
        // Prompt user for input
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        // Remove the trailing newline character
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Exit command check
        if (strcmp(cmd_buff, EXIT_CMD) == 0)
        {
            free(cmd_buff);
            return EXIT_SUCCESS;
        }

        // Dragon command check
        if (strcmp(cmd_buff, "dragon") == 0)
        {
            printf("%s", DRAGON_IMAGE);
            continue;
        }

        //Building Command List
        int rc = build_cmd_list(cmd_buff, &clist);
        if (rc == OK)
        {
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++)
            {
                if (strlen(clist.commands[i].args) > 0)
                {
                    printf("<%d> %s [%s]\n", i + 1, clist.commands[i].exe, clist.commands[i].args);
                }
                else
                {
                    printf("<%d> %s\n", i + 1, clist.commands[i].exe);
                }
            }
        }
        else if (rc == ERR_TOO_MANY_COMMANDS)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        }
        else if (rc == WARN_NO_CMDS)
        {
            printf(CMD_WARN_NO_CMD);
        }
    }
    free(cmd_buff);
    return EXIT_SUCCESS;
}
