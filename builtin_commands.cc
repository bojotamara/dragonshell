#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "builtin_commands.h"
#include "util.h"

/**
 * @brief Prints the current working directory to stdout
 */
void printWorkingDirectory() {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd))) {
        std::cout << cwd << std::endl;
    } else {
        std::cerr << "dragonshell: Unknown error\n";
    }
}

/**
 * @brief Changes the current directory
 * 
 * @param dir - Name of the directory
 */
void changeDirectory(std::string dir) {
    if (chdir(dir.c_str()) != 0) {
        std::cerr << "dragonshell: No such file or directory" << std::endl;
    }
}

/**
 * @brief Exits the shell and cleans up child processes
 */
void exitShell() {
    std::cout << "Exiting\n";
    for(std::size_t i = 0; i < childPids.size(); i++) {
        // Suspended processes won't respond to SIGTERM, so they must be continued
        kill(childPids[i], SIGTERM);
        kill(childPids[i], SIGCONT);
        waitpid(childPids[i], NULL, 0);
    }
    _exit(0);
}

/**
 * @brief Prints the current path held at G_PATH to the stdout
 */
void printPath() {
    std::string output = "Current PATH: ";
    for (std::size_t i = 0; i < G_PATH.size(); i++) {
        if (i != 0) {
        output += ":";
        }
        output += G_PATH[i];
    }
    std::cout << output << std::endl;
}

/**
 * @brief Append to G_PATH or overwrite it 
 * 
 * @param path - Path to append or overwrite
 */
void appendToPath(std::string path = "") {
    std::vector<std::string> paths = tokenize(path, ":");
    
    if (paths.empty()) {
        G_PATH.clear();
    } else if (paths[0].compare("$PATH") == 0) {
        for (std::size_t i = 1; i < paths.size(); i++) {
            G_PATH.push_back(paths[i]);
        }
    } else {
        G_PATH.clear();
        for (std::size_t i = 0; i < paths.size(); i++) {
            G_PATH.push_back(paths[i]);
        }
    }
}

/**
 * @brief Run a built-in command
 * 
 * @param arguments - Command split into arguments
 * @param redirectFile - Filename to redirect output to. Empty if no redirect desired
 */
void runBuiltInCommand(std::vector<std::string> arguments, std::string redirectFile) {
    // Separate out the command and the arguments
    std::string command = arguments[0];
    arguments.erase(arguments.begin());

    // Redirect the stdout to the file specified. Preserve it to restore later
    int stdout_copy = 0;
    if (!redirectFile.empty()) {
        dup2(STDOUT_FILENO, stdout_copy);
        int rfd = open(redirectFile.c_str(),  O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
        dup2(rfd, STDOUT_FILENO);
        close(rfd);
    }
  
    if(command.compare("cd") == 0) {
        if (arguments.size() == 0) {
            std::cerr << "dragonshell: expected argument to \"cd\"\n";
        } else if (arguments.size() > 1) {
            std::cerr << "dragonshell: \"cd\" only expects 1 argument\n";
        } else {
            changeDirectory(arguments[0]);
        }
    } else if (command.compare("pwd") == 0) {
        if (arguments.size() > 0) {
            std::cerr << "dragonshell: no expected argument to \"pwd\"\n";
        } else {
            printWorkingDirectory();
        }
    } else if (command.compare("exit") == 0) {
        if (arguments.size() > 0) {
            std::cerr << "dragonshell: no expected argument to \"exit\"\n";
        } else {
            exitShell();
        }
    } else if (command.compare("a2path") == 0) {
        if (arguments.size() > 1) {
            std::cerr << "dragonshell: \"a2path\" only expects 1 argument\n";
        } else if (arguments.size() == 0) {
            appendToPath();
        } else {
            appendToPath(arguments[0]);
        }
    } else if (command.compare("$PATH") == 0) {
        if (arguments.size() > 0) {
            std::cerr << "dragonshell: no expected argument to \"$PATH\"\n";
        } else {
        printPath();
        }
    }

    // Restore the stdout
    if (!redirectFile.empty()) {
        dup2(stdout_copy, STDOUT_FILENO);
    }
}

