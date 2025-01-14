#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50

// prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);

// prototypes for functions to handle required functionality
int count_words(char *, int, int);
// add additional prototypes here

int setup_buff(char *buff, char *user_str, int len)
{
    // Check if user string is bigger than buffer size
    if (len > BUFFER_SZ)
    {
        return -1;
    }

    // Pointers to iterate through the strings
    char *buffer_ptr = buff;
    char *user_str_ptr = user_str;

    int buffer_index = 0;
    int consecutive_whitespace = 0;

    // Iterate through the user string
    while (*user_str_ptr != '\0' && buffer_index < BUFFER_SZ - 1)
    {
        char currentCharacter = *user_str_ptr;
        // Handle any whitespaces encountered
        if (currentCharacter == ' ' || currentCharacter == '\t')
        {
            if (consecutive_whitespace > 0)
            {
                user_str_ptr++;
                continue;
            }
            else
            {
                *buffer_ptr = currentCharacter;
                consecutive_whitespace++;
                buffer_ptr++;
                buffer_index++;
            }
        }
        else
        {
            // Handle Non-white spaces. Reset consecutive whitespaces as well.
            *buffer_ptr = currentCharacter;
            buffer_ptr++;
            buffer_index++;
            consecutive_whitespace = 0;
        }

        user_str_ptr++;
    }

    // Fill remaining buffer space with periods
    while (buffer_index < BUFFER_SZ - 1)
    {
        *buffer_ptr = '.';
        buffer_ptr++;
        buffer_index++;
    }

    // Null-terminate the buffer
    *buffer_ptr = '\0';

    return buffer_index;
}

void print_buff(char *buff, int len)
{
    printf("Buffer:  ");
    for (int i = 0; i < len; i++)
    {
        putchar(*(buff + i));
    }
    putchar('\n');
}

void usage(char *exename)
{
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

int count_words(char *buff, int len, int str_len)
{
    // int wordCount = 0;
    // for (int i = 0; i < str_len; i++) {
    //     if (buff[i] == " ") {
    //         wordCount += 1;
    //     }
    // }

    // printf("Word Count: %i", wordCount);
    return 0;
}

// ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[])
{

    char *buff;         // placehoder for the internal buffer
    char *input_string; // holds the string provided by the user on cmd line
    char opt;           // used to capture user option from cmd line
    int rc;             // used for return codes
    int user_str_len;   // length of user supplied string

    // TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //   If arv[1] does not exist, then there is no flag/argument given that specifices the type of command that wants to be run. This condition allows the program to gracefully error when this argument is not appropriately given.
    if ((argc < 2) || (*argv[1] != '-'))
    {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1); // get the option flag

    // handle the help flag and then exit normally
    if (opt == 'h')
    {
        usage(argv[0]);
        exit(0);
    }

    // WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    // TODO:  #2 Document the purpose of the if statement below
    //  This if statement checks if there is a string is provided in the command. If a string is not provided, then an error should/will be thrown.
    if (argc < 3)
    {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; // capture the user input string

    // TODO:  #3 Allocate space for the buffer using malloc and
    //           handle error if malloc fails by exiting with a
    //           return code of 99
    //  CODE GOES HERE FOR #3
    buff = malloc(sizeof(BUFFER_SZ));
    if (buff == NULL)
    {
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ); // see todos
    if (user_str_len < 0)
    {
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt)
    {
    case 'c':
        rc = count_words(buff, BUFFER_SZ, user_str_len); // you need to implement
        if (rc < 0)
        {
            printf("Error counting words, rc = %d", rc);
            exit(2);
        }
        printf("Word Count: %d\n", rc);
        break;

    // TODO:  #5 Implement the other cases for 'r' and 'w' by extending
    //        the case statement options
    case 'r':

        // case 'w:':

    default:
        usage(argv[0]);
        exit(1);
    }

    // TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    exit(0);
}

// TODO:  #7  Notice all of the helper functions provided in the
//           starter take both the buffer as well as the length.  Why
//           do you think providing both the pointer and the length
//           is a good practice, after all we know from main() that
//           the buff variable will have exactly 50 bytes?
//
//           PLACE YOUR ANSWER HERE
