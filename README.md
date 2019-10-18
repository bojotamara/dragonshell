# Dragonshell

Dragonshell is a simple interactive shell written in C++. The shell  supports a small number of built-in commands
and has the ability to run external commands. It supports features like piping, output redirection, and background 
processing.

### Running the shell
Compile the project and run the shell.
```sh
$ make
$ ./dragonshell
```
### Design Choices
The shell was designed with separation of concerns and modularity in mind. Each file deals with a different aspect
of the shell and they interface with each other through one or two functions. This was done so that if one file or
module changes, it doesn't impact the rest of the code.
The code is divided into 4 main files: `dragonshell.cc`, `builtin_commands.cc`, `external_commands.cc`, and 
`util.cc`.

###### dragonshell.cc
This file contains the main shell loop, where input is read. The command is parsed and split into individual commands. 
The commands are identified as built-in or external, and whether or not piping and output redirection is needed is 
determined. It is then passed on to the specific command handlers, `builtin_commands.cc` or `external_commands.cc`.
This file also deals with the signal handling part of the requirements. The SIGINT and SIGTSTP signals are caught 
by a handler function that only prints a new line. This makes use of the fact that every child process will call `execve`. 
When it is called, the signal handler of the external program is used instead. That way, the child process will receive 
the SIGINT and SIGTSTP signal and deal with it as normal, while the shell ignores these signals.

###### builtin_commands.cc
This file handles built-in commands. Built-in commands don't need to be run in a child process, so no forking occurs 
here. All of the supported commands have a separate function that executes them. The entry point for this file is a 
general function that verifies the input, prints error messages if the commands are not used correctly, and runs 
the individual command once identified.

###### external_commands.cc
This files handles the running of external programs. Before a program is run, a child process is created to run the 
program in. Given the name of the program, this file attempts to execute the program at every possible path, by 
running `execve` and seeing if it fails. If the loop iterates through all the paths and if no program is executed, 
then the program cannot be found. The parent process waits for the child to finish execution using `waitpid` unless 
the command is specified as a background job.
The handling of piped commands also occurs in this file. The piped commands undergo the same treatment as running an 
individual, external program, except a pipe is created to communicate between the two.

###### util.cc
This file contains functions that are used in the other files. It also contains the two global variables that are 
used, `childPids` and `G_PATH`. The first is a vector that holds the PIDs of every child process that needs to be 
cleaned up on exit. This includes background processes, and processes interrupted with SIGINT or SIGTSTP. The 
second contains the searchable paths for external programs.

### Features + System Calls
| Feature                       | System Calls Used                        |
| ----------------------------- |:----------------------------------------:|
| (1.a) cd                      | `chdir()`                                |
| (1.b) pwd                     | `getcwd()`                               |
| (1.c) a2path                  | None                                     |
| (1.d) exit                    | `_exit()` `kill()` `waitpid()`           |
| (2) External programs         | `fork()` `execve()` `waitpid()` `_exit()`|
| (3) Running multiple commands | None                                     |
| (4) Background execution      | `fork()` `execve()` `dup2()` `close()`   |
| (5) Output redirection        | `dup2()` `open()` `close()`              |
| (6) Piping                    | `pipe()` `dup2()` `close()`              |
| (7) Signal Handling           | `sigaction()`                            |

### Testing Strategy
Each feature was tested during and after its implementation. I did a form of dirty testing, where I would try every 
possible input to see if I could encounter any errors. I would input as many possibilities as I could think of, in 
order to try and break the program.
After more features were implemented, I did a sort of integration testing, where I would combine command types with 
one another (eg. output redirection with piping) and see how the system would react. Again, I would input as many 
different combinations and possibilites as I could think of.
As a final sanity test, the examples in the assignment description were used to test the basic functionality. 
Then, some edge cases and clarifications were tested by running examples from the discussion forum.

### Sources
No external sources, other than class notes, were used.
