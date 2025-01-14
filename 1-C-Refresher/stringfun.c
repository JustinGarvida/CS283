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
int reverse_string(char *buff, int len, int str_len);
int print_words(char *buff, int len, int str_len);
// add additional prototypes here

int setup_buff(char *buff, char *user_str, int len)
// TODO: #4: Implement the setup buff as per the directions
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
    int word_count = 0;
    int currently_word = 0; // indicates if currently in a word or not
    char *user_str_ptr = buff;
    while (*user_str_ptr != '\0' && *user_str_ptr != '.')
    {
        char currentCharacter = *user_str_ptr;
        if (currentCharacter == ' ' || currentCharacter == '\t')
        {
            currently_word = 0;
        }
        else if (!currently_word)
        {
            currently_word = 1;
            word_count++;
        }
        user_str_ptr++;
    }
    return word_count;
}

int reverse_string(char *buff, int len, int str_len)
{
    char *word_start = buff;
    char *word_end = buff;
    while (*word_end != '.' && *word_end != '\0')
    {
        word_end++;
    }
    word_end--;
    // Print Reversed String
    printf("Reversed String: ");
    while (word_end >= word_start)
    {
        putchar(*word_end);
        word_end--;
    }
    printf("\n");
    return 0;
}

int print_words(char *buff, int len, int str_len)
{
    char *curr = buff;
    int word_count = 1;
    int character_count = 0;
    int currently_word = 0;

    printf("Word Print\n");
    printf("----------\n");

    // Store start of current word
    char *word_start = curr;

    while (*curr != '.' && *curr != '\0')
    {
        if (*curr == ' ' || *curr == '\t')
        {
            if (currently_word)
            {
                // Print the completed word
                printf("%d. ", word_count);
                char *temp = word_start;
                while (temp < curr)
                {
                    putchar(*temp);
                    temp++;
                }
                printf(" (%d)\n", character_count);

                word_count++;
                currently_word = 0;
                character_count = 0;
            }
        }
        else
        {
            if (!currently_word)
            {
                word_start = curr;
                currently_word = 1;
            }
            character_count++;
        }
        curr++;
    }

    // Print last word if exists
    if (currently_word)
    {
        printf("%d. ", word_count);
        char *temp = word_start;
        while (temp < curr)
        {
            putchar(*temp);
            temp++;
        }
        printf(" (%d)\n", character_count);
    }
    return 0;
}

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

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0)
    {
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt)
    {
    case 'c':
        rc = count_words(buff, BUFFER_SZ, user_str_len);
        if (rc < 0)
        {
            printf("Error counting words, rc = %d", rc);
            exit(2);
        }
        printf("Word Count: %d\n", rc);
        break;

    case 'r':
        rc = reverse_string(buff, BUFFER_SZ, user_str_len);
        if (rc < 0)
        {
            printf("Error reversing string, rc = %d", rc);
            exit(2);
        }
        break;

    case 'w':
        rc = print_words(buff, BUFFER_SZ, user_str_len);
        if (rc < 0)
        {
            printf("Error printing words, rc = %d", rc);
            exit(2);
        }
        break;

    default:
        usage(argv[0]);
        exit(1);
    }

    // TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);
    exit(0);
}

// TODO:  #7  Notice all of the helper functions provided in the
//           starter take both the buffer as well as the length.  Why
//           do you think providing both the pointer and the length
//           is a good practice, after all we know from main() that
//           the buff variable will have exactly 50 bytes?
//
//           It is a good idea to provide both the pointer and the length because it allows us to prevent out of range errors when accessing the pointer. 
//           Additionally, these arguments allow us to do sanity checks before running the program, ensuring it handles any erorrs gracefully.