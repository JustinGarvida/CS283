In this assignment I suggested you use fgets() to get user input in the main while loop. Why is fgets() a good choice for this application?

Answer: start here

You needed to use malloc() to allocte memory for cmd_buff in dsh_cli.c. Can you explain why you needed to do that, instead of allocating a fixed-size array?

Answer: start here

In dshlib.c, the function build_cmd_list()` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

Answer: start here

For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google "linux shell stdin stdout stderr explained" to get started.

One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

Answer: start here

You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

Answer: start here

STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

Answer: start here

How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

Answer: start here