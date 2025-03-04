#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// Structure to hold command and its arguments
typedef struct
{
    char **args;
} Command;

void execute_pipeline(Command commands[], int num_commands)
{
    int pipes[num_commands - 1][2]; // Array of pipes
    pid_t pids[num_commands];       // Array to store process IDs

    // Create all necessary pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            exit(ERR_EXEC_CMD);
        }
    }

    // Create processes for each command
    for (int i = 0; i < num_commands; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            exit(ERR_EXEC_CMD);
        }

        if (pids[i] == 0)
        { // Child process
            if (i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }
            if (i < num_commands - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            fflush(stdout); // Ensure output is properly flushed before exec
            execvp(commands[i].args[0], commands[i].args);
            exit(ERR_EXEC_CMD);
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < num_commands; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
}

int exec_piped_commands(command_list_t *clist)
{
    int num_cmds = clist->num;
    Command commands[num_cmds];

    for (int i = 0; i < num_cmds; i++)
    {
        commands[i].args = clist->commands[i].argv;
    }

    pid_t supervisor = fork();
    if (supervisor == -1)
    {
        return ERR_EXEC_CMD;
    }

    if (supervisor == 0)
    { // Supervisor process
        execute_pipeline(commands, num_cmds);
        exit(EXIT_SUCCESS);
    }

    waitpid(supervisor, NULL, 0);
    return OK;
}

int exec_local_cmd_loop()
{
    cmd_buff_t cmd_buff;
    int rc = alloc_cmd_buff(&cmd_buff);
    if (rc != OK)
    {
        return ERR_MEMORY;
    }

    while (1)
    {
        if (fgets(cmd_buff._cmd_buffer, SH_CMD_MAX, stdin) == NULL)
        {
            break;
        }
        cmd_buff._cmd_buffer[strcspn(cmd_buff._cmd_buffer, "\n")] = '\0';

        // Parse the command buffer into command list (handling pipes)
        command_list_t clist;
        rc = build_cmd_list(cmd_buff._cmd_buffer, &clist);
        if (rc != OK)
        {
            return rc;
        }

        // Execute commands with piping
        rc = exec_piped_commands(&clist);
        if (rc != OK)
        {
            return rc;
        }
    }

    free_cmd_buff(&cmd_buff);
    return OK;
}

Built_In_Cmds match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);
    switch (cmd_type)
    {
    case BI_CMD_EXIT:
        return OK_EXIT;
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

int alloc_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (!cmd_buff)
    {
        return ERR_MEMORY;
    }
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer)
    {
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (!cmd_buff || !cmd_buff->_cmd_buffer)
    {
        return ERR_MEMORY;
    }
    free(cmd_buff->_cmd_buffer);
    cmd_buff->_cmd_buffer = NULL;
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (!cmd_line || !clist)
    {
        return ERR_MEMORY;
    }
    char *token = strtok(cmd_line, "|");
    int count = 0;

    while (token != NULL)
    {
        while (*token == ' ')
            token++;
        if (strlen(token) >= ARG_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        clist->commands[count].argv[0] = strtok(token, " ");
        int i = 1;
        while ((clist->commands[count].argv[i] = strtok(NULL, " ")) != NULL)
        {
            i++;
        }
        clist->commands[count].argv[i] = NULL;
        count++;

        token = strtok(NULL, "|");
        if (count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }
    }
    clist->num = count;
    return OK;
}