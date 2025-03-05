#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

int exec_local_cmd_loop()
{
    char *cmd_buff;
    command_list_t clist;
    int rc = 0;

    // Allocate memory for the command buffer
    cmd_buff = malloc(SH_CMD_MAX);
    if (!cmd_buff)
    {
        fprintf(stderr, "DEBUG: Failed to allocate memory for cmd_buff\n");
        return ERR_MEMORY;
    }
    printf("DEBUG: Successfully allocated cmd_buff\n");

    while (1)
    {
        printf("%s", SH_PROMPT);
        fflush(stdout);

        // Read user input
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        printf("DEBUG: User entered: %s\n", cmd_buff);

        // Remove newline character from input
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        printf("DEBUG: Trimmed input: %s\n", cmd_buff);
        clist.num = 0;

        // Tokenize input using pipes
        char *eachPipedCommand = strtok(cmd_buff, PIPE_STRING);
        while (eachPipedCommand != NULL)
        {
            printf("DEBUG: Processing piped command: %s\n", eachPipedCommand);

            if (clist.num >= CMD_MAX)
            {
                fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
                continue;
            }

            // Allocate command buffer
            rc = alloc_cmd_buff(&clist.commands[clist.num]);
            if (rc != OK)
            {
                fprintf(stderr, "DEBUG: Failed to allocate command buffer at index %d\n", clist.num);
                return ERR_MEMORY;
            }

            // Parse command
            rc = build_cmd_buff(eachPipedCommand, &clist.commands[clist.num]);
            if (rc == WARN_NO_CMDS)
            {
                fprintf(stderr, CMD_WARN_NO_CMD);
                break;
            }
            printf("DEBUG: Command parsed: %s\n", clist.commands[clist.num]._cmd_buffer);

            clist.num++;
            eachPipedCommand = strtok(NULL, PIPE_STRING);
        }

        if (clist.num == 0)
        {
            printf("DEBUG: No valid commands, skipping iteration.\n");
            continue;
        }

        if (clist.num == 1)
        {
            Built_In_Cmds cmd_type = match_command(clist.commands[0].argv[0]);
            printf("DEBUG: Matched command type: %d\n", cmd_type);

            if (cmd_type != BI_NOT_BI)
            {
                if (cmd_type == BI_CMD_EXIT)
                {
                    printf("DEBUG: Exit command detected. Exiting shell.\n");
                    free(cmd_buff);
                    return OK_EXIT;
                }
                else if (cmd_type == BI_CMD_CD)
                {
                    if (clist.commands[0].argc > 1)
                    {
                        printf("DEBUG: Changing directory to %s\n", clist.commands[0].argv[1]);
                        if (chdir(clist.commands[0].argv[1]) != 0)
                        {
                            perror("DEBUG: cd failed");
                        }
                    }
                    continue;
                }
                exec_built_in_cmd(&clist.commands[0]);
            }
            else
            {
                printf("DEBUG: Executing external command\n");
                exec_cmd(&clist.commands[0]);
            }
        }
        else
        {
            printf("DEBUG: Executing pipeline of %d commands\n", clist.num);
            execute_pipeline(&clist);
        }
    }

    free(cmd_buff);
    return OK;
}

int exec_cmd(cmd_buff_t *cmd)
{
    printf("DEBUG: Forking process to execute: %s\n", cmd->argv[0]);

    int process_id = fork();
    if (process_id == -1)
    {
        perror("DEBUG: Fork failed");
        return ERR_EXEC_CMD;
    }
    else if (process_id == 0)
    {
        printf("DEBUG: In child process, executing: %s\n", cmd->argv[0]);
        fflush(stdout);
        if (execvp(cmd->argv[0], cmd->argv) == -1)
        {
            perror("DEBUG: execvp failed");
            exit(ERR_EXEC_CMD);
        }
    }
    else
    {
        int status;
        waitpid(process_id, &status, 0);
        printf("DEBUG: Process %d completed with status %d\n", process_id, status);
    }
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || !cmd_buff || !cmd_buff->_cmd_buffer)
    {
        fprintf(stderr, "DEBUG: Null command buffer detected\n");
        return ERR_MEMORY;
    }

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    cmd_buff->argc = 0;

    char *current_pointer = cmd_buff->_cmd_buffer;
    bool in_string = false;

    while (*current_pointer)
    {
        current_pointer = skip_spaces(current_pointer);
        if (*current_pointer == '\0')
            break;

        cmd_buff->argv[cmd_buff->argc++] = parse_argument(current_pointer, &in_string);
        while (*current_pointer)
            current_pointer++;

        current_pointer++;
    }
    cmd_buff->argv[cmd_buff->argc] = NULL;

    printf("DEBUG: Parsed %d arguments.\n", cmd_buff->argc);
    for (int i = 0; i < cmd_buff->argc; i++)
    {
        printf("DEBUG: Arg[%d]: %s\n", i, cmd_buff->argv[i]);
    }

    return (cmd_buff->argc > 0) ? OK : WARN_NO_CMDS;
}

int execute_pipeline(command_list_t *clist)
{
    int num_commands = clist->num;
    int pipes[num_commands - 1][2];
    pid_t pids[num_commands];

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("DEBUG: Pipe creation failed");
            return ERR_EXEC_CMD;
        }
    }

    // Create processes for each command
    for (int i = 0; i < num_commands; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("DEBUG: Fork failed in pipeline execution");
            return ERR_EXEC_CMD;
        }

        if (pids[i] == 0) // Child process
        {
            // Redirect input from previous pipe (if not first command)
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("DEBUG: dup2 failed for input");
                    exit(ERR_EXEC_CMD);
                }
            }

            // Redirect output to next pipe (if not last command)
            if (i < num_commands - 1)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("DEBUG: dup2 failed for output");
                    exit(ERR_EXEC_CMD);
                }
            }

            // Close all pipes in child process
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            printf("DEBUG: Executing piped command: %s\n", clist->commands[i].argv[0]);
            fflush(stdout);

            if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) == -1)
            {
                perror("DEBUG: execvp failed in pipeline");
                exit(ERR_EXEC_CMD);
            }
        }
    }

    // Parent process closes all pipe ends
    for (int i = 0; i < num_commands - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes and check exit status
    int overall_status = OK;
    for (int i = 0; i < num_commands; i++)
    {
        int status;
        if (waitpid(pids[i], &status, 0) == -1)
        {
            perror("DEBUG: waitpid failed");
            overall_status = ERR_EXEC_CMD;
        }
        else if (WIFEXITED(status) && WEXITSTATUS(status) != OK)
        {
            printf("DEBUG: Piped process %d exited with status %d\n", pids[i], WEXITSTATUS(status));
            overall_status = ERR_EXEC_CMD;
        }
    }

    return overall_status;
}
