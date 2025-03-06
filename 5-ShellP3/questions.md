1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation ensures that all child processes complete before the shell continues accepting user input by calling waitpid() on each spawned process. This preventz zomvie processes and ensures proper synchronization between commands. Without waitpid(), child processes could remain in a zombie state, leading to resource leaks and unpredictable behavior.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

After using dup2() to redirect input/output, it is necessary to close unused pipe ends to avoid resource leaks and blocking issues. If a process keeps the write end of a pipe open, the reading process may hang indefinitely, waiting for input that never arrives. Leaving pipes open can also lead to file descriptor exhaustion, causing failures in subsequent commands.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command is implemented as a built-in because it modifies the shell’s working directory, which an external process cannot do persistently. If cd were an external command, the directory change would only apply to the child process and not affect the shell. This would make navigation ineffective, requiring a workaround like sourcing a script to apply changes.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To support an arbitrary number of piped commands, I would dynamically allocate memory for command structures instead of using a fixed-size array. This could be achieved with malloc() and realloc() or a linked list, ensuring efficient memory usage. However, this increases implementation complexity and may introduce additional overhead due to dynamic memory management.