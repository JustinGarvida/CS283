1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  We would use fork/execvp instead of just calling execvp directly because execvp replaces the current running process with a new process, which ends the orignal process. Fork creats a child process that duplicates the parent, then the child can call execvp without ending the parent.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork() system call fails, it returns a -1. My implementation checks the return value and then prints and error message to stderr if an error is found.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: execvp() finds the command to eecute by looking through the directories in the PATH variable. It Will keep going through each directory until a matching command is found.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The purpose of calling wait() in the parent process after forking is that it allows the child to finish. If it is not called, the child process wouldn't be terminated properly.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() provides the exit status code of the child process and returns it in wait. This is imporant because it allows us to see if the child was terminated correctly.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  My build_cmd_bugg() handles quotes argumnets by processing te text in quotes as just one argument. This is necessary because spaving can be important within the quotes for certain commands. 

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  One change that I had to make was processing any quotation marks encountered in the program. One unexpected challenge I had was ensuring that the whitespaces were kept in the quotations.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  The purose of signals in a Linux system is that they are notifications sent to a certain process/thread that indicates if an event has happened. Unlike other IPC methods, signals do not carry complex data

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL is used to forcefully terminate a process immedately. This can be used when you want to end a program instantly upon a certain command. SIGTERM politely asks a process to terminate. This can be used to gracefully shut down a program. SIGINT interrupts a process. This can be used to interrupt a ongoing program

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives SIGSTOP, it is immediately stopped. It cannot be caught or ignored like SIGINT. This is the case because it allows the OS to maintain ongoing jobs.