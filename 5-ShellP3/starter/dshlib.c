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
    cmd_buff->argv[cmd_buff->argc] = NULL;
    if (cmd_buff->argc > 0)
    {
        return OK;
    }
    else
    {
        return WARN_NO_CMDS;
    }
}

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
