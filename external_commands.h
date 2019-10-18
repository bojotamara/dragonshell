#pragma once

enum PipeDirection {PIPE_IN, PIPE_OUT, NO_PIPE};

void runExternalCommand(std::vector<std::string> arguments, std::string redirectFile = "", PipeDirection pipeDirection = NO_PIPE, int pipefd[] = NULL);
void runPipedCommands(std::string rawCommand1, std::string rawCommand2);