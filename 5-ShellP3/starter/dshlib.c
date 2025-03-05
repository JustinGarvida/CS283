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
    int rc = 0;
    // TODO IMPLEMENT MAIN LOOP

    // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff

    // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
    // the cd command should chdir to the provided directory; if no directory is provided, do nothing

    // TODO IMPLEMENT if not built-in command, fork/exec as an external command
    // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"
    cmd_buff = malloc(SH_CMD_MAX);
    if (!cmd_buff)
    {
        free(cmd_buff);
        return ERR_MEMORY;
    }
    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        clist.num = 0;
        char *eachPipedCommand = strtok(cmd_buff, PIPE_STRING);
        while (eachPipedCommand != NULL)
        {
            if (clist.num >= CMD_MAX)
            {
                return ERR_TOO_MANY_COMMANDS;
            }
            int commandBufferReturnCode = alloc_cmd_buff(&clist.commands[clist.num]);
            if (commandBufferReturnCode != 0)
            {
                return ERR_MEMORY;
            }
            rc = build_cmd_buff(eachPipedCommand, &clist.commands[clist.num]);
            if (rc == WARN_NO_CMDS)
            {
                printf(CMD_WARN_NO_CMD);
                break;
            }
            clist.num++;
            eachPipedCommand = strtok(NULL, PIPE_STRING);
        }
        if (clist.num == 0)
        {
            continue;
        }
        if (clist.num > CMD_MAX)
        {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }
        if (clist.num == 1)
        {
            if (exec_built_in_cmd(&clist.commands[0]) == BI_NOT_BI)
            {
                exec_cmd(&clist.commands[0]);
            }
        }
        else
        {
            execute_pipeline(&clist);
        }
    }
    free(cmd_buff);
    return OK;
}

int alloc_cmd_buff(cmd_buff_t *cmd_buff)
{
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer)
    {
        return OK;
    }
    return ERR_MEMORY;
}

int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    free(cmd_buff->_cmd_buffer);
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    int concurrentQuotes = 0;
    cmd_buff->argc = 0;
    strcpy(cmd_buff->_cmd_buffer, cmd_line);
    char *inputCommandLine = cmd_buff->_cmd_buffer;
    while (*inputCommandLine != '\0' && cmd_buff->argc < CMD_MAX)
    {
        while (*inputCommandLine == SPACE_CHAR)
        {
            inputCommandLine++;
        }
        if (*inputCommandLine == '"')
        {
            concurrentQuotes = 1;
            inputCommandLine++;
        }
        if (*inputCommandLine == '\0')
        {
            break;
        }
        cmd_buff->argv[cmd_buff->argc] = inputCommandLine;
        cmd_buff->argc++;
        while (*inputCommandLine != '\0')
        {
            if (concurrentQuotes == 1)
            {
                if (*inputCommandLine == '"')
                {
                    concurrentQuotes = 0;
                    *inputCommandLine = '\0';
                    inputCommandLine++;
                    break;
                }
            }
            else
            {
                if (*inputCommandLine == SPACE_CHAR)
                {
                    *inputCommandLine = '\0';
                    inputCommandLine++;
                    break;
                }
            }
            inputCommandLine++;
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
    Built_In_Cmds enumeratedCommandCode = match_command(cmd->argv[0]);
    switch (enumeratedCommandCode)
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
    int processReturnCode;
    int processID = fork();
    if (processID < 0)
    {
        return ERR_EXEC_CMD;
    }
    else if (processID == 0)
    {
        if (execvp(cmd->argv[0], cmd->argv) == -1)
        {
            return ERR_EXEC_CMD;
        }
    }
    else
    {
        waitpid(processID, &processReturnCode, 0);
        return WEXITSTATUS(processReturnCode);
    }
    return OK;
}

int execute_pipeline(command_list_t *clist)
{
    int numberOfCommands = clist->num;
    int fileReadWriteConditions = 2;
    int numberOfPipes[numberOfCommands - 1][fileReadWriteConditions];
    pid_t allChildProcessIDs[numberOfCommands];
    for (int eachPipe = 0; eachPipe < numberOfCommands - 1; eachPipe++)
    {
        if (pipe(numberOfPipes[eachPipe]) == -1)
        {
            return ERR_EXEC_CMD;
        }
    }
    for (int eachCommand = 0; eachCommand < numberOfCommands; eachCommand++)
    {
        pid_t newProcessID = fork();
        if (newProcessID == -1)
        {
            return ERR_EXEC_CMD;
        }
        allChildProcessIDs[eachCommand] = newProcessID;
        if (newProcessID == 0)
        {
            if (eachCommand > 0)
            {
                dup2(numberOfPipes[eachCommand - 1][0], STDIN_FILENO);
            }
            if (eachCommand < numberOfCommands - 1)
            {
                dup2(numberOfPipes[eachCommand][1], STDOUT_FILENO);
            }
            for (int eachPipe = 0; eachPipe < numberOfCommands - 1; eachPipe++)
            {
                close(numberOfPipes[eachPipe][0]);
                close(numberOfPipes[eachPipe][1]);
            }
            execvp(clist->commands[eachCommand].argv[0], clist->commands[eachCommand].argv);
            exit(ERR_EXEC_CMD);
        }
    }
    for (int eachPipe = 0; eachPipe < numberOfCommands - 1; eachPipe++)
    {
        close(numberOfPipes[eachPipe][0]);
        close(numberOfPipes[eachPipe][1]);
    }
    for (int eachChildProcess = 0; eachChildProcess < numberOfCommands; eachChildProcess++)
    {
        int processReturnCode;
        waitpid(allChildProcessIDs[eachChildProcess], &processReturnCode, 0);
    }
    return OK;
}