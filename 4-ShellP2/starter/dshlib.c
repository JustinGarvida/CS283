#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

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

        // Parse command and arguments FIRST
        rc = build_cmd_buff(cmd_buff._cmd_buffer, &cmd_buff);
        if (rc == WARN_NO_CMDS)
        {
            fprintf(stderr, CMD_WARN_NO_CMD);
            continue;
        }
        else if (rc == ERR_MEMORY)
        {
            fprintf(stderr, "Error: Command buffer memory allocation failed\n");
            break;
        }

        // Match built-in commands using parsed argv[0]
        Built_In_Cmds cmd_type = match_command(cmd_buff.argv[0]);

        // Handle Built-In Commands
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

        // Handle External Commands
        rc = exec_cmd(&cmd_buff);
        if (rc != OK)
        {
            // fprintf(stderr, CMD_ERR_EXECUTE);
        }
    }

    free_cmd_buff(&cmd_buff);
    return OK;
}

int exec_cmd(cmd_buff_t *cmd)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "Error: Failed to fork process\n");
        return ERR_EXEC_CMD;
    }
    else if (pid == 0) // Child process
    {
        if (execvp(cmd->argv[0], cmd->argv) == -1)
        {
            perror("execvp"); // Print execution failure message
            exit(EXIT_FAILURE);
        }
    }
    else // Parent process
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            fprintf(stderr, "Error: Failed to wait for child process\n");
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

    return BI_NOT_BI; // Not a built-in command
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds cmd_type = match_command(cmd->argv[0]);

    switch (cmd_type)
    {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;

    case BI_CMD_DRAGON:
        print_dragon();
        return BI_EXECUTED;

    case BI_CMD_CD:
        if (cmd->argc > 1)
        {
            if (chdir(cmd->argv[1]) != 0)
            {
                perror("cd failed");
                return ERR_EXEC_CMD;
            }
        }
        else
        {
            fprintf(stderr, "cd: missing argument\n");
            return ERR_EXEC_CMD;
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

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || !cmd_buff || !cmd_buff->_cmd_buffer)
    {
        return ERR_MEMORY;
    }

    strncpy(cmd_buff->_cmd_buffer, cmd_line, SH_CMD_MAX - 1);
    cmd_buff->_cmd_buffer[SH_CMD_MAX - 1] = '\0';
    cmd_buff->argc = 0;

    char *ptr = cmd_buff->_cmd_buffer;
    bool in_quotes = false;

    while (*ptr)
    {
        // Skip leading spaces
        while (*ptr == ' ' && !in_quotes)
            ptr++;

        if (*ptr == '\0')
            break;

        // Check for quoted string
        if (*ptr == '"')
        {
            in_quotes = true;
            ptr++; // Move past the opening quote
        }

        cmd_buff->argv[cmd_buff->argc++] = ptr;

        // Move to the next space or closing quote
        while (*ptr && (in_quotes || *ptr != ' '))
        {
            if (*ptr == '"')
            {
                *ptr = '\0'; // Null-terminate and remove the ending quote
                in_quotes = false;
                break;
            }
            ptr++;
        }

        if (*ptr)
        {
            *ptr = '\0'; // Null terminate this argument
            ptr++;
        }
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;

    return (cmd_buff->argc > 0) ? OK : WARN_NO_CMDS;
}
