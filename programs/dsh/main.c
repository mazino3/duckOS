/*
    This file is part of duckOS.

    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.

    Copyright (c) Byteduck 2016-2020. All rights reserved.
*/

//A simple shell.

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

char cwd[4096];
char cmdbuf[4096];
struct stat st;

void evaluate_input(char* input);
pid_t evaluate_command(int argc, char** argv, int infd, int outfd);
bool evaluate_builtin(int argc, char** argv);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main() {
	while(true) {
		getcwd(cwd, 4096);
		printf("[dsh %s]# ", cwd);
		fflush(stdout);
		gets(cmdbuf);
		evaluate_input(cmdbuf);
	}
}
#pragma clang diagnostic pop

/**
 * @param input The input to be evaluated.
 */
void evaluate_input(char* input) {
	//Split commands by pipe delimiter
	int cmdc = 0;
	char* commands[512];
	char* cmd = strtok(input, "|");
	commands[cmdc++] = cmd;
	while((cmd = strtok(NULL, "|"))) {
		commands[cmdc++] = cmd;
	}

	int pids[256];
	int pidc = 0;

	//Evaluate commands
	int prev_pipe_in = -1;
	for(int i = 0; i < cmdc; i++) {
		//If this isn't the last command, create a new pipe
		int outfd = -1;
		if(i != cmdc - 1) {
			int pipefd[2];
			pipe(pipefd);
			outfd = pipefd[1];
			prev_pipe_in = pipefd[0];
		}

		//Split arguments
		int argc = 0;
		char* args[512];
		char* arg = strtok(commands[i], " ");
		args[argc++] = arg;
		while((arg = strtok(NULL, " "))) {
			args[argc++] = arg;
		}

		//If this isn't a builtin command, run the command
		if(!evaluate_builtin(argc, args))
			pids[pidc++] = evaluate_command(argc, args, prev_pipe_in, outfd);
	}

	//Wait for processes
	for(int i = 0; i < pidc; i++)
		waitpid(pids[i], NULL, 0);
}

pid_t evaluate_command(int argc, char** argv, int infd, int outfd) {
		pid_t pid = fork();
		if(!pid){
			if(infd != -1)
				dup2(infd, STDIN_FILENO);
			if(outfd != -1)
				dup2(outfd, STDOUT_FILENO);

			execvp(argv[0], argv);
			perror("Cannot execute");
			exit(errno);
		}
		return pid;
}

bool evaluate_builtin(int argc, char** argv) {
	char* cmd = argv[0];
	if(!strcmp(cmd, "exit")) {
		exit(0);
	} else if(!strcmp(cmd, "cd")) {
		errno = 0;
		if(argc < 2) printf("No directory specified.\n");
		else if(chdir(argv[1])) perror("Could not change directory");
		return true;
	}
	return false;
}