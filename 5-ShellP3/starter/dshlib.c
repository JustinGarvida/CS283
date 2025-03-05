#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

void print_dragon();

int alloc_cmd_buff(cmd_buff_t *cmd_buff)
{
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);

    if (cmd_buff->_cmd_buffer == NULL)
    {
        return ERR_MEMORY;
    }

    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    free(cmd_buff->_cmd_buffer);

    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    cmd_buff->argc = 0;
    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);

    char *ptr = cmd_buff->_cmd_buffer;
    bool quotes = false;

    while (*ptr != '\0' && cmd_buff->argc < CMD_MAX)
    {
        while (*ptr == SPACE_CHAR)
        {
            ptr++;
        }

        if (*ptr == '\0')
        {
            break;
        }

        if (*ptr == QUOTE_CHAR)
        {
            quotes = true;
            ptr++;
        }

        cmd_buff->argv[cmd_buff->argc] = ptr;
        cmd_buff->argc++;

        while (*ptr != '\0')
        {
            if (quotes)
            {
                if (*ptr == QUOTE_CHAR)
                {
                    *ptr = '\0';
                    ptr++;
                    quotes = false;
                    break;
                }
            }
            else
            {
                if (*ptr == SPACE_CHAR)
                {
                    *ptr = '\0';
                    ptr++;
                    break;
                }
            }
            ptr++;
        }
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;

    if (cmd_buff->argc == 0)
    {
        return WARN_NO_CMDS;
    }

    return OK;
}

Built_In_Cmds match_command(const char *input)
{
    if (strcmp(input, EXIT_CMD) == 0)
    {
        return BI_CMD_EXIT;
    }
    else if (strcmp(input, "dragon") == 0)
    {
        return BI_CMD_DRAGON;
    }
    else if (strcmp(input, "cd") == 0)
    {
        return BI_CMD_CD;
    }

    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);

    switch (cmd_type)
    {
    case BI_CMD_EXIT:
        exit(OK_EXIT);
    case BI_CMD_DRAGON:
        print_dragon();
        return BI_CMD_DRAGON;
    case BI_CMD_CD:
        if (cmd->argc > 1)
        {
            chdir(cmd->argv[1]);
        }
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}

int exec_cmd(cmd_buff_t *cmd)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        if (execvp(cmd->argv[0], cmd->argv) == -1)
        {
            fprintf(stderr, CMD_ERR_EXECUTE);
        }
        exit(ERR_EXEC_CMD);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        return ERR_EXEC_CMD;
    }
    else
    {
        fprintf(stderr, "error: fork failed.\n");
        return ERR_EXEC_CMD;
    }
}

int execute_pipeline(command_list_t *clist)
{
    int num_cmds = clist->num;
    int pipes[num_cmds - 1][2];
    pid_t pids[num_cmds];

    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    for (int i = 0; i < num_cmds; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("fork");
            return ERR_EXEC_CMD;
        }

        if (pids[i] == 0)
        {
            if (i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < num_cmds - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_cmds; i++)
    {
        waitpid(pids[i], NULL, 0);
    }

    return OK;
}

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 *
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 *
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    char *cmd_buff;
    command_list_t clist;
    int rc;

    cmd_buff = malloc(SH_CMD_MAX);
    if (cmd_buff == NULL)
    {
        fprintf(stderr, "error: memory allocation failed.\n");
        return ERR_MEMORY;
    }

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        clist.num = 0;

        char *token = strtok(cmd_buff, PIPE_STRING);
        while (token != NULL)
        {
            if (clist.num >= CMD_MAX)
            {
                clist.num++;
                break;
            }

            if (alloc_cmd_buff(&clist.commands[clist.num]) != OK)
            {
                fprintf(stderr, "error: memory allocation failed.\n");
                return ERR_MEMORY;
            }

            rc = build_cmd_buff(token, &clist.commands[clist.num]);
            if (rc == WARN_NO_CMDS)
            {
                printf(CMD_WARN_NO_CMD);
                break;
            }

            clist.num++;
            token = strtok(NULL, PIPE_STRING);
        }

        if (clist.num > CMD_MAX)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }

        if (clist.num == 0)
        {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (clist.num > 1)
        {
            execute_pipeline(&clist);
        }
        else
        {
            if (exec_built_in_cmd(&clist.commands[0]) == BI_NOT_BI)
            {
                exec_cmd(&clist.commands[0]);
            }
        }

        for (int i = 0; i < clist.num; i++)
        {
            free_cmd_buff(&clist.commands[i]);
        }
    }

    free(cmd_buff);
    return OK;
}