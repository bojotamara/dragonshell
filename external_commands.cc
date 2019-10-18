#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "external_commands.h"
#include "util.h"

// Exit status when no command was found in the searchable paths
const int COMMAND_NOT_FOUND = 20;

/**
 * @brief Get the paths that should be searched for an external program
 * 
 * @param command - The external program
 * @return std::vector<std::string> - The list of paths to search for an executable
 */
std::vector<std::string> getPathsToSearch(std::string command) {
    std::vector<std::string> pathsToSearch;
    bool isPathProvided = command.at(0) == '/';
  
    if (isPathProvided) {
        // Search for command in path provided
        pathsToSearch.push_back(command);
    } else {
        // Search for command in $PATH and current working directory
        pathsToSearch.push_back("./" + command);
        for (std::size_t i = 0; i < G_PATH.size(); i++) {
            std::string path = G_PATH[i];
            if (path.back() != '/') {
                path += '/';
            }
            pathsToSearch.push_back(path + command);
        }
    }
    return pathsToSearch;
}

/**
 * @brief Execute an external program
 * 
 * @param path - The path where the program is located
 * @param arguments - The actual command/program and its arguments
 */
void execExternalCommand(std::string path, std::vector<std::string> arguments) {
    char *argv1[arguments.size()+1];
    for (size_t i = 0; i < arguments.size(); i++) {
        argv1[i] = (char *)arguments[i].c_str();
    }
    argv1[arguments.size()] = NULL;
    if (execve(path.c_str(), argv1, {NULL}) == -1) {
        return;
    } 
}

/**
 * @brief Pipe the output of one command to the input of another
 * 
 * @param rawCommand1 - The first command that sends output to rawCommand2
 * @param rawCommand2 - The second command that receives input from rawCommand1
 */
void runPipedCommands(std::string rawCommand1, std::string rawCommand2) {
    int fd[2];
    if (pipe(fd) < 0) {
        std::cerr << "dragonshell: Piping failed\n";
    }
    // See if command requires output redirection
    std::string redirectFile1 = getRedirectFile(&rawCommand1);
    std::string redirectFile2 = getRedirectFile(&rawCommand2);

    std::vector<std::string> command1 = tokenize(rawCommand1, " ");
    std::vector<std::string> command2 = tokenize(rawCommand2, " ");
    
    runExternalCommand(command1, redirectFile1, PipeDirection::PIPE_OUT, fd);
    runExternalCommand(command2, redirectFile2, PipeDirection::PIPE_IN, fd);
}

/**
 * @brief Run an external command in a forked child
 * 
 * @param arguments - The command and its arguments
 * @param redirectFile - Filename to redirect output to. Empty if no redirect desired
 * @param pipeDirection - Whether the command needs to pipe in, pipe out, or not pipe at all
 * @param pfd - The file descriptors for the pipe, in the case of a piped command
 */
void runExternalCommand(std::vector<std::string> arguments, std::string redirectFile, PipeDirection pipeDirection, int pipefd[]) {
    int status = -1;
    pid_t pid;

    std::vector<std::string> pathsToSearch = getPathsToSearch(arguments[0]);
    bool isBackgroundJob = arguments.back() == "&";

    pid = fork();
    if (pid < 0) {
        std::cerr << "dragonshell: Fork failed\n";
    } else if (pid == 0) {
        if (!redirectFile.empty()) {
            // Redirect output of command to a file, if provided
            int rfd = open(redirectFile.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
            dup2(rfd, STDOUT_FILENO);
            close(rfd);
        } else if (pipeDirection == PIPE_OUT) {
            // Send output to another command through a pipe
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        } 
        
        // Receive input from another command through a pipe
        if (pipeDirection == PIPE_IN) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
        }

        // Supress output of background processes
        if (isBackgroundJob) {
            int fd = open("/dev/null", O_RDWR);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            arguments.pop_back();
        }

        // For every possible path, attempt to execute the command
        for(auto it = pathsToSearch.begin(); it != pathsToSearch.end(); ++it) {
            std::string path = *it;
            execExternalCommand(path, arguments);
        }
        _exit(COMMAND_NOT_FOUND); // if this is reached, no command was executed
    }
  
    if (pipeDirection == PIPE_IN) {
        close(pipefd[0]);
    } else if (pipeDirection == PIPE_OUT) {
        close(pipefd[1]);
    }

    if (!isBackgroundJob) {
        waitpid(pid, &status, 0);
        // If command was terminated, add it to list for cleanup upon exit
        if (status == -1) {
            childPids.push_back(pid);
        }
    } else {
        // Add background process to list for cleanup
        childPids.push_back(pid);
        std::cout << "PID ";
        std::cout << pid;
        std::cout << " is running in the background\n";
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) == COMMAND_NOT_FOUND) {
        std::cerr << "dragonshell: Command not found\n";
    }
}
