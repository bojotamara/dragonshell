#include <vector>
#include <string>
#include <cstring>
#include <set>
#include <algorithm>

#include "util.h"

std::vector<std::string> G_PATH = {"/bin/", "/usr/bin/"};
std::vector<pid_t> childPids;

/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return std::vector<std::string> - The list of tokenized strings. Can be empty
 */
std::vector<std::string> tokenize(const std::string &str, const char *delim) {
    char* cstr = new char[str.size() + 1];
    std::strcpy(cstr, str.c_str());

    char* tokenized_string = strtok(cstr, delim);

    std::vector<std::string> tokens;
    while (tokenized_string != NULL)
    {
        tokens.push_back(std::string(tokenized_string));
        tokenized_string = strtok(NULL, delim);
    }
    delete[] cstr;

    return tokens;
}

/**
 * @brief Identify a command as built-in or external
 * 
 * @param str - The command to identify
 * @return CmdType - The command type
 */
CmdType identifyCmd(std::string command) {
    std::set<std::string> commands = {"cd", "pwd", "a2path", "exit", "$PATH"};
    if (commands.find(command) != commands.end()) {
        return CmdType::BUILT_IN;
    }
    return CmdType::EXTERNAL;
}

/**
 * @brief Determine if a command requires an ouput redirect to a file. Change the 
 * command string to remove the file reference.
 * 
 * @param rawCommand - Pointer to the original command
 * @return std::string - File to redirect output to. Empty if no redirect desired
 */
std::string getRedirectFile(std::string *rawCommand) {
    std::vector<std::string> tokens = tokenize(*rawCommand, ">");
    std::string redirectFile = "";
    if (tokens.size() == 2) {
        std::vector<std::string> file = tokenize(tokens[1], " ");
        if (file.size() == 1) {
            *rawCommand = tokens[0];
            redirectFile = file[0];
        }
    } 
    return redirectFile;
}