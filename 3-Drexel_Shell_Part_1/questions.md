In this assignment I suggested you use fgets() to get user input in the main while loop. Why is fgets() a good choice for this application?

Answer: fgets() is a good choide for this application because it prevents buffer overflow. This allows the program to deny any commands that are too long by limiting the number of characters read.

You needed to use malloc() to allocte memory for cmd_buff in dsh_cli.c. Can you explain why you needed to do that, instead of allocating a fixed-size array?

Answer: You need to use malloc() in this scenario because malloc() allows the program to dynamically create arrays based on the size of the command string rather than the max size of the command string from the program. This allows us to save and manage memory efficently.

In dshlib.c, the function build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

Answer: This is necessary because it allows us to ensure the only part of the string that we should be processing is the actual word/command itself. If we didn't trim space, there could be issues with compilation and evaluation of the string.

For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google "linux shell stdin stdout stderr explained" to get started.

One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

Answer: One redirection example that we should implement in our shell is redirecting any output into a file. A challenge that could come with this is properly writing to a file and formatting it in the proper way. Another redirection example is redirecting STDIN from a file. THis could be useful in the future because it would allow our program to process files and do any actions we need them to (i.e moving, deleting, etc.). The challenging part would be ensuring that the file contents are not changed in this process. Lastly, a good example would be redirecting STDERR to a file. This would be good because it allows users to log errors from the program. A struggle could be ensuring that despite an error, that the error can be logged properly.

You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

Answer: Some key differences between redirection and piping are their purpose and how they handle data flow. Redirection involves sending input or output to or from files or devices, modifying how a command interacts with external resources. In contrast, piping connects multiple commands, allowing the output of one command to become the input of the next, facilitating real-time data processing between commands. Redirection works on static input or output, while piping is dynamic and enables command chaining.

STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

Answer: It is important to keep STDERR and STDOUT separate because they serve different purposes. STDOUT is used for regular, expected output, while STDERR is reserved for error messages. Separating them allows users and scripts to distinguish between successful output and errors, enabling better handling of each. For example, a script can process STDOUT without being disrupted by error messages, and errors can be redirected or logged independently for easier debugging and troubleshooting. This separation improves both automation and user experience in the shell.

How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

Answer: Our custom shell should handle errors by displaying STDERR separately from STDOUT to distinguish regular output from errors. It should also check the exit status of commands and show relevant error messages. Additionally, the shell should offer an option to merge STDOUT and STDERR, using redirection like 2>&1, for cases where users need combined output. This approach ensures clear feedback and flexibility.