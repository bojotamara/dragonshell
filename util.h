#pragma once

extern std::vector<std::string> G_PATH;
extern std::vector<pid_t> childPids;

enum CmdType {EXTERNAL, BUILT_IN};

std::vector<std::string> tokenize(const std::string &str, const char *delim);
CmdType identifyCmd(std::string command);
std::string getRedirectFile(std::string *rawCommand);