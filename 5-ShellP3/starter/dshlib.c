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

// Helper function to allocate memory for a command buffer
int allocate_command_buffer(cmd_buff_t *cmd_buff)
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

// Helper function to free memory for a command buffer
int free_command_buffer(cmd_buff_t *cmd_buff)
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

// Helper function to parse a command line into arguments
int parse_command_line(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || !cmd_buff || !cmd_buff->_cmd_buffer)
    {
        return ERR_MEMORY;
    }

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    cmd_buff->argc = 0;

    char *current_ptr = cmd_buff->_cmd_buffer;
    bool in_string = false;
    while (*current_ptr)
    {
        // Skip leading spaces
        while (*current_ptr == ' ')
        {
            current_ptr++;
        }
        if (*current_ptr == '\0')
        {
            break;
        }

        // Handle quoted arguments
        if (*current_ptr == '"')
        {
            in_string = true;
            current_ptr++;
        }

        cmd_buff->argv[cmd_buff->argc++] = current_ptr;

        // Find the end of the argument
        while (*current_ptr)
        {
            if (in_string)
            {
                if (*current_ptr == '"')
                {
                    *current_ptr = '\0';
                    in_string = false;
                    current_ptr++;
                    break;
                }
            }
            else
            {
                if (*current_ptr == ' ')
                {
                    *current_ptr = '\0';
                    current_ptr++;
                    break;
                }
            }
            current_ptr++;
        }
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;

    if (cmd_buff->argc == 0)
    {
        return WARN_NO_CMDS;
    }
    return OK;
}

// Helper function to execute a single command
int execute_single_command(cmd_buff_t *cmd)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        return ERR_EXEC_CMD;
    }
    else if (pid == 0)
    {
        // Child process: execute the command
        if (execvp(cmd->argv[0], cmd->argv) == -1)
        {
            return ERR_EXEC_CMD;
        }
    }
    else
    {
        // Parent process: wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            return WEXITSTATUS(status);
        }
        return ERR_EXEC_CMD;
    }
}

// Helper function to execute a pipeline of commands
int execute_command_pipeline(command_list_t *clist)
{
    if (clist == NULL || clist->num == 0)
    {
        return WARN_NO_CMDS;
    }

    int num_commands = clist->num;
    int pipes[num_commands - 1][2]; // Array to hold pipe file descriptors
    pid_t pids[num_commands];       // Array to store process IDs

    // Create pipes for the pipeline
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    // Fork processes for each command in the pipeline
    for (int i = 0; i < num_commands; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("fork");
            return ERR_EXEC_CMD;
        }

        if (pids[i] == 0) // Child process
        {
            // Redirect input from the previous pipe (if not the first command)
            if (i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Redirect output to the next pipe (if not the last command)
            if (i < num_commands - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe file descriptors in the child process
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute the command
            if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) == -1)
            {
                perror("execvp");
                exit(ERR_EXEC_CMD);
            }
        }
    }

    // Parent process: close all pipe file descriptors
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++)
    {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != OK)
        {
            return ERR_EXEC_CMD;
        }
    }

    return OK;
}

Built_In_Cmds match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    return BI_NOT_BI;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds command_inputted = match_command(cmd->argv[0]);
    switch (command_inputted)
    {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;

    case BI_CMD_DRAGON:
        print_dragon();
        return BI_EXECUTED;
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

// Main shell loop
int exec_local_cmd_loop()
{
    char cmd_buff[SH_CMD_MAX]; // Buffer to store user input
    command_list_t clist;      // Command list to store parsed commands
    int rc;

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0'; // Remove trailing newline

        // Parse the command line into a list of commands
        rc = build_cmd_list(cmd_buff, &clist);
        if (rc == WARN_NO_CMDS)
        {
            fprintf(stderr, CMD_WARN_NO_CMD);
            continue;
        }
        else if (rc == ERR_TOO_MANY_COMMANDS)
        {
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }
        else if (rc != OK)
        {
            fprintf(stderr, "Error: Failed to parse command\n");
            continue;
        }

        // Handle built-in commands (only if there's a single command)
        if (clist.num == 1)
        {
            Built_In_Cmds cmd_type = match_command(clist.commands[0].argv[0]);
            if (cmd_type != BI_NOT_BI)
            {
                Built_In_Cmds exec_result = exec_built_in_cmd(&clist.commands[0]);
                if (exec_result == BI_CMD_EXIT)
                {
                    return OK_EXIT;
                }
                continue;
            }
        }

        // Execute the pipeline of commands
        rc = execute_command_pipeline(&clist);
        if (rc != OK)
        {
            // fprintf(stderr, CMD_ERR_EXECUTE);
        }
    }

    return OK;
}