#include <vector>
#include <string>
#include <unistd.h>
#include <iostream>
#include <signal.h>

#include "util.h"
#include "builtin_commands.h"
#include "external_commands.h"

/**
 * @brief Signal handler that only prints a new line
 */
void signalHandler(int signum) {
	std::cout << std::endl;
}

/**
 * @brief Block the SIGINT and SIGTSTP signals by assigning a handler.
 */
void blockSignals() {
    struct sigaction sa;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = signalHandler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
}

/**
 * @brief Run the command identified in the string. This function funnels the command to
 * the correct handling function
 * 
 * @param string - The raw command string
 */
void runCommand(std::string rawCommand) {
    // First, separate into individual commands based on "|"
    std::vector<std::string> pipingCommands = tokenize(rawCommand, "|");
    if (pipingCommands.size() == 2) {
        runPipedCommands(pipingCommands[0], pipingCommands[1]);
        return;
    } else if (pipingCommands.size() > 2) {
        std::cerr << "dragonshell: Pipe chains are not supported\n";
        return;
    }

    // See if command requires output redirection
    std::string redirectFile = getRedirectFile(&rawCommand);

    // Split command up into arguments separated by spaces
    std::vector<std::string> arguments = tokenize(rawCommand, " ");
    CmdType cmdType = identifyCmd(arguments[0]);

    if (cmdType == BUILT_IN) {
        runBuiltInCommand(arguments, redirectFile);
    } else if (cmdType == EXTERNAL){ 
        runExternalCommand(arguments, redirectFile, PipeDirection::NO_PIPE);
    }
}

int main(int argc, char **argv) {
    // print the string prompt without a newline, before beginning to read
    // tokenize the input, run the command(s), and print the result
    // do this in a loop
    std::string input;

    std::cout << "Welcome to Dragon Shell!\n";

    blockSignals();

    while(true) {
        std::cout << ("dragonshell > ");

        std::getline(std::cin, input);
        
        // If CTRL-D is pressed at the prompt, we want to quit the shell
        if (std::cin.eof() && feof(stdin)) {
            std::cout << std::endl;
            exitShell();
        }
    
        // In the case of an interrupt, clear the error state so we can 
        // receive input again
        if (std::cin.fail()) {
            std::cin.clear();
        }

        std::vector<std::string> commands = tokenize(input, ";");
        for(auto it = commands.begin(); it != commands.end(); ++it) {
            std::string command = *it;
            runCommand(command);
        }
    }

    return 0;
}