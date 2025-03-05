#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

// Function to skip leading whitespace in a string
char *skip_whitespace(char *str)
{
    while (*str && isspace((unsigned char)*str))
    {
        str++;
    }
    return str;
}

// Function to handle quoted strings in command parsing
char *handle_quoted_string(char *str, cmd_buff_t *cmd_buff)
{
    str++; // Skip the opening quote
    cmd_buff->argv[cmd_buff->argc++] = str;

    while (*str && *str != '"')
    {
        str++;
    }

    if (*str == '"')
    {
        *str = '\0'; // Null-terminate the quoted string
        str++;
    }

    return str;
}

// Function to handle unquoted strings in command parsing
char *handle_unquoted_string(char *str, cmd_buff_t *cmd_buff)
{
    cmd_buff->argv[cmd_buff->argc++] = str;

    while (*str && !isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str)
    {
        *str = '\0'; // Null-terminate the argument
        str++;
    }

    return str;
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

// Function to allocate memory for the command buffer
int alloc_cmd_buff(cmd_buff_t *cmd_buff)
{
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer)
    {
        return ERR_MEMORY;
    }
    memset(cmd_buff, 0, sizeof(cmd_buff_t));
    return OK;
}

// Function to free memory for the command buffer
int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (cmd_buff->_cmd_buffer)
    {
        free(cmd_buff->_cmd_buffer);
    }
    return OK;
}

// Function to clear the command buffer
int clear_cmd_buff(cmd_buff_t *cmd_buff)
{
    memset(cmd_buff, 0, sizeof(cmd_buff_t));
    return OK;
}

// Function to build the command buffer from a command line
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || strlen(cmd_line) >= SH_CMD_MAX)
    {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    cmd_buff->argc = 0;

    char *str = cmd_buff->_cmd_buffer;
    while (*str)
    {
        str = skip_whitespace(str);

        if (*str == '\0')
        {
            break;
        }

        if (*str == '"')
        {
            str = handle_quoted_string(str, cmd_buff);
        }
        else
        {
            str = handle_unquoted_string(str, cmd_buff);
        }

        if (cmd_buff->argc >= CMD_ARGV_MAX)
        {
            return ERR_TOO_MANY_COMMANDS;
        }
    }

    cmd_buff->argv[cmd_buff->argc] = NULL; // Null-terminate argv
    return OK;
}

// Function to execute piped commands
int execute_pipeline(command_list_t *clist)
{
    int num_commands = clist->num;
    int pipes[num_commands - 1][2]; // Array of pipes
    pid_t pids[num_commands];       // Array to store process IDs

    // Create all necessary pipes
    for (int i = 0; i < num_commands - 1; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return ERR_EXEC_CMD; // Return error code
        }
    }

    // Create processes for each command
    for (int i = 0; i < num_commands; i++)
    {
        pids[i] = fork();
        if (pids[i] == -1)
        {
            perror("fork");
            return ERR_EXEC_CMD; // Return error code
        }

        if (pids[i] == 0)
        { // Child process
            // Set up input pipe for all except first process
            if (i > 0)
            {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Set up output pipe for all except last process
            if (i < num_commands - 1)
            {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipe ends in child
            for (int j = 0; j < num_commands - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command using argv from cmd_buff_t
            if (execvp(clist->commands[i].argv[0], clist->commands[i].argv) == -1)
            {
                perror("execvp");
                exit(ERR_EXEC_CMD); // Exit with error code
            }
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
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != OK)
        {
            return ERR_EXEC_CMD; // Return error code if any child fails
        }
    }

    return OK; // Return success
}

// Main command loop
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
        rc = execute_pipeline(&cmd_list);
        if (rc != OK)
        {
            // fprintf(stderr, CMD_ERR_EXECUTE);
        }
    }

    free_cmd_buff(&cmd_buff);
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || clist == NULL)
    {
        return WARN_NO_CMDS;
    }
    memset(clist, 0, sizeof(command_list_t));
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
        while (*command == SPACE_CHAR)
        {
            command++;
        }
        if (strlen(command) >= ARG_MAX)
        {
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        char *space_pos = strchr(command, SPACE_CHAR);
        clist->commands[command_count]._cmd_buffer = malloc(ARG_MAX + EXE_MAX);
        if (!clist->commands[command_count]._cmd_buffer)
        {
            return ERR_MEMORY;
        }

        if (space_pos != NULL)
        {
            size_t exe_len = space_pos - command;
            if (exe_len >= EXE_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            clist->commands[command_count].argv[0] = clist->commands[command_count]._cmd_buffer;
            strncpy(clist->commands[command_count]._cmd_buffer, command, exe_len);
            clist->commands[command_count]._cmd_buffer[exe_len] = '\0';
            clist->commands[command_count].argc = 1;

            space_pos++;
            while (*space_pos == SPACE_CHAR)
            {
                space_pos++;
            }

            if (*space_pos != '\0')
            {
                clist->commands[command_count].argv[1] = clist->commands[command_count]._cmd_buffer + exe_len + 1;
                strncpy(clist->commands[command_count]._cmd_buffer + exe_len + 1, space_pos, ARG_MAX);
                clist->commands[command_count]._cmd_buffer[exe_len + 1 + ARG_MAX - 1] = '\0';
                clist->commands[command_count].argc++;
            }
        }
        else
        {
            if (strlen(command) >= EXE_MAX)
            {
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            clist->commands[command_count].argv[0] = clist->commands[command_count]._cmd_buffer;
            strncpy(clist->commands[command_count]._cmd_buffer, command, EXE_MAX);
            clist->commands[command_count]._cmd_buffer[EXE_MAX - 1] = '\0';
            clist->commands[command_count].argc = 1;
        }

        command_count++;
        command = strtok(NULL, PIPE_STRING);
    }

    clist->num = command_count;
    return OK;
}
