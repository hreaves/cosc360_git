// COSC360 Lab8 Jshell
// Harrison Reaves
// This program is a small and primitive shell that executes commands and redirects their input/output
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fields.h"
#include "jrb.h"
#include "jval.h"
#include "dllist.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

typedef struct {
    char *stdin;          /* Filename from which to redirect stdin.  NULL if empty. */
    char *stdout;         /* Filename to which to redirect stdout.  NULL if empty. */
    int append_stdout;    /* Boolean for appending. */
    int wait;             /* Boolean for whether I should wait. */
    int n_commands;       /* The number of commands that I have to execute */
    int *argcs;           /* argcs[i] is argc for the i-th command */
    char ***argvs;        /* argcv[i] is the argv array for the i-th command */
    Dllist comlist;       /* I use this to incrementally read the commands. */
} Command;

// free all memory of command when finished executing
void free_cmd(Command *cmd) {
	Dllist tmp;
	if(cmd ==  NULL) {return;}
	if(cmd->stdin != NULL) {free(cmd->stdin);}
	if(cmd->stdout != NULL) {free(cmd->stdout);}
	if (cmd->argvs != NULL) {
        for (int i = 0; i < cmd->n_commands; i++) {
            for (int j = 0; cmd->argvs[i][j] != NULL; j++) {
				free(cmd->argvs[i][j]);
			}
		    free(cmd->argvs[i]);

		}
		free(cmd->argvs);
    }
	if (cmd->argcs != NULL) {free(cmd->argcs);}
	if (cmd->comlist != NULL) {free_dllist(cmd->comlist);}
}
// executing command after reading END
void execute(Command *cmd) { 
	int pid, return_pid, next = 1, status, pipefd[2], prev_fd = -1;
	JRB pids, tmp;
	pids = make_jrb();
	for(int i = 0; i < cmd->n_commands; i++) {
		fflush(stdin);
		fflush(stdout);
		fflush(stderr);
		if(i == cmd->n_commands - 1) {next = 0;} // tells if current command has a next command
		// if it has a next command, create a pipe
		if(next) {
			if (pipe(pipefd) < 0) {
                perror("pipe");
                exit(1);
			}
		}
		pid = fork();
		// for child only, redirecting input and output for each process through the pipes using dup2
		if(pid == 0) {
			// if first process, input comes from stdin
			if(i == 0) {
				if(cmd->stdin != NULL) {
                    int fd = open(cmd->stdin, O_RDONLY);
                    if (fd < 0) {
                        perror(cmd->stdin);
                        exit(1);
                    }
                    if (dup2(fd, 0) < 0) {
                        perror("dup2");
                        exit(1);
                    }
                    close(fd);
                }
			}
			// else, input comes from previous command's pipe
			else {
				if(dup2(prev_fd, 0) < 0) {
					perror("dup2");
					exit(1);
				}
				close(prev_fd);
			}
			// if it has a following process, output goes to the pipe
			if(next) {
				if(dup2(pipefd[1], 1) < 0) {
					perror("dup2");
                    exit(1);
				}
				close(pipefd[0]);
				close(pipefd[1]);
			}
			// if last process, output goes to stdout
			else {
				if(cmd->stdout != NULL) {
                    int fd;
                    if (cmd->append_stdout) {fd = open(cmd->stdout, O_WRONLY | O_CREAT | O_APPEND, 0644);}
                    else {fd = open(cmd->stdout, O_WRONLY | O_CREAT | O_TRUNC, 0644);}
                    if (fd < 0) {
                        perror(cmd->stdout);
                        exit(1);
                    }
                    if (dup2(fd, 1) < 0) {
                        perror("dup2");
                        exit(1);
                    }
                    close(fd);
				}	
			}
			// execute the process
			execvp(cmd->argvs[i][0], cmd->argvs[i]);
			perror(cmd->argvs[i][0]);
			exit(1);
		}

		if (i > 0) {close(prev_fd);}
		// since the pipe will get reused, save the read end of the pipe for the next process to use as input
		if(next){
			close(pipefd[1]);
			prev_fd = pipefd[0];
		}
		jrb_insert_int(pids, pid, new_jval_i(0));
	}
	// if waiting for each process to end, add all pids to a jrb tree and wait. Everytime a child returns, find and remove
	// from tree until tree is empty, and then continue forth.
	if(cmd->wait) {
		while(!jrb_empty(pids)) {
			return_pid = wait(&status);
			if(return_pid < 0) {
				perror("wait");
				break;
			}
			tmp = jrb_find_int(pids, return_pid);
			if(tmp != NULL) {
				jrb_delete_node(tmp);
			}
		}
	}
	jrb_free_tree(pids);
}

int main() {

	IS is;
	is = new_inputstruct(NULL);

	Command *cmd = malloc(sizeof(Command));
	// Loop infinitely reading every set of commands
	while(1) {
		// initialize the command variable every new set of commands
	    int file_end = 0;
		cmd->append_stdout = 0;
		cmd->wait = 1;
		cmd->n_commands = 0;
		cmd->argcs = NULL;
		cmd->argvs = NULL;
		cmd->stdin = NULL;
		cmd->stdout = NULL;
		cmd->comlist = new_dllist();
		// read each part of the command and store in Dllist until see word END
		while(get_line(is) >= 0) {
			if (is->NF == 0 || is->fields[0][0] == '#') {continue;}
			// once END is seen, allocate memory for array of argvs and argcs
			if (strcmp(is->fields[0], "END") == 0) {
				file_end = 1;
				cmd->argvs = (char***) malloc(sizeof(char**) * cmd->n_commands);
				cmd->argcs = (int*) malloc(sizeof(int) * cmd->n_commands);
				Dllist tmp;
				int i = 0;
				// go through dllist and fill in the argvs and argcs arrays
				dll_traverse(tmp, cmd->comlist) {
					char **argv = tmp->val.v;
					cmd->argvs[i] = argv;
					int argc = 0;
					while(argv[argc] != NULL) {
						argc++;
					}
					cmd->argcs[i] = argc;
					i++;
				}
				// once filled, execute the command
				if(cmd->n_commands > 0) {execute(cmd);}
				free_cmd(cmd);
				break;
			}
			// these next three are not commands but simply redirect input and output of command
			else if (strcmp(is->fields[0], "<") == 0) {cmd->stdin = strdup(is->fields[1]);}
			else if (strcmp(is->fields[0], ">") == 0) {cmd->stdout = strdup(is->fields[1]);}
			else if (strcmp(is->fields[0], ">>") == 0) {
				cmd->stdout = strdup(is->fields[1]);
				cmd->append_stdout = 1;	
			}
			else if (strcmp(is->fields[0], "NOWAIT") == 0) {cmd->wait = 0;}
			// take each word of line and store in array and add the command to the dllist for later
			else {
				char **argv = malloc(sizeof(char *)*(is->NF + 1));
				for(int i = 0; i < is->NF; i++) {
					argv[i] = strdup(is->fields[i]);
				}
				argv[is->NF] = NULL;
				dll_append(cmd->comlist, new_jval_v(argv));
				cmd->n_commands++;
			}
		}
		// if reading commands from file, quit program at end of file
		if (!file_end) {
			free_cmd(cmd);
			break;
		}
	}
	jettison_inputstruct(is);
	free(cmd);
	return 0;
}
