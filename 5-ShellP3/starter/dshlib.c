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
    cmd_buff_t cmd_buff;
    int rc = alloc_cmd_buff(&cmd_buff);
    if (rc != OK)
    {
        fprintf(stderr, "Error: Failed to allocate memory\n");
        return ERR_MEMORY;
    }

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff._cmd_buffer, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        cmd_buff._cmd_buffer[strcspn(cmd_buff._cmd_buffer, "\n")] = '\0';

        // Handle built-in commands directly
        if (strcmp(cmd_buff._cmd_buffer, EXIT_CMD) == 0)
        {
            free_cmd_buff(&cmd_buff);
            return OK_EXIT;
        }
        if (strcmp(cmd_buff._cmd_buffer, "dragon") == 0)
        {
            printf("%s", DRAGON_IMAGE);
            continue;
        }

        // Parse the command line into a list of commands
        command_list_t cmd_list;
        rc = build_cmd_list(cmd_buff._cmd_buffer, &cmd_list);
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
            fprintf(stderr, "Error: Command parsing failed\n");
            continue;
        }

        // Handle built-in commands
        Built_In_Cmds cmd_type = match_command(cmd_list.commands[0].argv[0]);
        if (cmd_type != BI_NOT_BI)
        {
            Built_In_Cmds exec_result = exec_built_in_cmd(&cmd_buff);
            if (exec_result == BI_CMD_EXIT)
            {
                free_cmd_buff(&cmd_buff);
                return OK_EXIT;
            }
            continue;
        }

        // Execute piped commands
        rc = exec_piped_commands(&cmd_list);
        if (rc != OK)
        {
            // fprintf(stderr, CMD_ERR_EXECUTE);
        }
    }
    free_cmd_buff(&cmd_buff);
    return OK;
}

int exec_piped_commands(command_list_t *cmd_list)
{
    int num_commands = cmd_list->num;
    int pipefds[2 * (num_commands - 1)];
    int status;
    pid_t pid;

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipefds + i * 2) < 0)
        {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }

    // Execute commands
    for (int i = 0; i < num_commands; i++)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return ERR_EXEC_CMD;
        }
        else if (pid == 0)
        {
            // Child process
            if (i != 0)
            {
                // Redirect stdin from the previous pipe
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (i != num_commands - 1)
            {
                // Redirect stdout to the next pipe
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (num_commands - 1); j++)
            {
                close(pipefds[j]);
            }

            // Execute the command using argv
            if (execvp(cmd_list->commands[i].argv[0], cmd_list->commands[i].argv) == -1)
            {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Close all pipe file descriptors in the parent
    for (int i = 0; i < 2 * (num_commands - 1); i++)
    {
        close(pipefds[i]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++)
    {
        wait(&status);
    }

    return OK;
}

int exec_cmd(cmd_buff_t *cmd)
{
    int process_id = fork();
    if (process_id == -1)
    {
        return ERR_EXEC_CMD;
    }
    else if (process_id == 0)
    {
        if (execvp(cmd->argv[0], cmd->argv) == -1)
        {
            return ERR_EXEC_CMD;
        }
    }
    else
    {
        int status;
        if (waitpid(process_id, &status, 0) == -1)
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

int clear_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (!cmd_buff || !cmd_buff->_cmd_buffer)
    {
        return ERR_MEMORY;
    }
    memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    return OK;
}

char *skip_spaces(char *string_pointer)
{
    while (*string_pointer == ' ')
        string_pointer++;
    return string_pointer;
}

char *parse_argument(char *string_pointer, bool *in_string)
{
    if (*string_pointer == '"')
    {
        *in_string = true;
        string_pointer++;
    }
    char *arg_start = string_pointer;
    while ((*string_pointer && *in_string) || (*string_pointer != ' '))
    {
        if (*string_pointer == '"')
        {
            *string_pointer = '\0';
            *in_string = false;
            break;
        }
        string_pointer++;
    }
    if (*string_pointer)
    {
        *string_pointer = '\0';
        string_pointer++;
    }
    return arg_start;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || !cmd_buff || !cmd_buff->_cmd_buffer)
    {
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
    cmd_buff->argv[cmd_buff->argc] = NULL; // Ensure argv is null-terminated
    if (cmd_buff->argc > 0)
    {
        return OK;
    }
    else
    {
        return WARN_NO_CMDS;
    }
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || clist == NULL)
    {
        return WARN_NO_CMDS;
    }

    // Initialize the command list
    memset(clist, 0, sizeof(command_list_t));

    // Tokenize the command line by pipes
    char *command = strtok(cmd_line, PIPE_STRING);
    int command_count = 0;

    if (command == NULL)
    {
        return WARN_NO_CMDS;
    }

    while (command != NULL)
    {
        if (command_count >= CMD_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Skip leading spaces in the command
        while (*command == SPACE_CHAR)
        {
            command++;
        }

        // Check if the command is too long
        if (strlen(command) >= SH_CMD_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        // Allocate memory for the command buffer
        int rc = alloc_cmd_buff(&clist->commands[command_count]);
        if (rc != OK)
        {
            return ERR_MEMORY;
        }

        // Copy the command into the buffer
        strncpy(clist->commands[command_count]._cmd_buffer, command, SH_CMD_MAX - 1);
        clist->commands[command_count]._cmd_buffer[SH_CMD_MAX - 1] = '\0';

        // Parse the command into arguments
        rc = build_cmd_buff(clist->commands[command_count]._cmd_buffer, &clist->commands[command_count]);
        if (rc != OK)
        {
            return rc;
        }

        command_count++;
        command = strtok(NULL, PIPE_STRING);
    }

    clist->num = command_count;
    return OK;
}