// Cade Lilley
// Lab03A - Simple shell
// Leaves no child processes behind

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		char* usr_bin_str = (char *)malloc(1000); // Creates a string to hold the bin directory + command file name
		strcpy(usr_bin_str, "/usr/bin/");

		if (tokens[0] == NULL) { // No input
			continue;
		} else if (!strcmp(tokens[0], "cd")) { // If command is cd, change parent cwd
			if(chdir(tokens[1])) {
				perror("cd failed");
			}
			char cwd[PATH_MAX];
   			getcwd(cwd, sizeof(cwd));
   			printf("Current Working Directory: %s\n", cwd);
   		} else if (!strcmp(tokens[0], "exit") || !strcmp(tokens[0], "Exit") || !strcmp(tokens[0], "EXIT")) { // If exit
   			exit(EXIT_SUCCESS); // TODO, kill zombie processes, make sure all memory is unallocated
		} else {
			// 0 = default fork, exec, wait
			// 1 (&) = fork, exec, run in background (single process only)
			// 2 (&&) = executed in sequence with wait
			// 3 (&&&) = executed in parellel in foreground
			int logic_option = 0;
			int index = 0;
			while(tokens[index]) {
				if(tokens[index][2] == '&')
					logic_option = 3;
				else if(tokens[index][1] == '&')
					logic_option = 2;
				else if(tokens[index][0] == '&')
					logic_option = 1;
				index++;
			}
			printf("Logic option chosen: %i\n", logic_option);

			// Command Execution
			
			// Used to keep track the current base command index, while the loop finds the parameters
			int base = 0; 
			// Keeps track of how far we've parsed the parameters from the base command. Terminated by a logic specifier (&,&&,&&&)
			int offset = 0;
			while(tokens[base]) { // While there are still tokens to check

				char **temp_command = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *)); // holds command splice from tokens

				while(tokens[base + offset]) {
					if(tokens[base + offset] && tokens[base + offset][0] != '&') // If we haven't hit a logic specifier
						temp_command[offset] = tokens[base + offset]; // "Append" the current token to the command we're building
 					if(!tokens[base + offset + 1] || tokens[base + offset][0] == '&') { // If the token is logic identifier or next token is end

						// Execute Command logic 
						if(access(strcat(usr_bin_str, tokens[base]), F_OK)) {
							printf("Command not found\n");
						} else if(logic_option == 0) { // 0 = default fork, exec, wait
							if (!fork())
								execvp(temp_command[0], temp_command);
							else
								wait(NULL);
						} else if(logic_option == 1) {  // 1 (&) = fork, exec, run in background (single process only)
							if (!fork())
								execvp(temp_command[0], temp_command);
						} else if(logic_option == 2) { // 2 (&&) = executed in sequence with wait
							if (!fork())
								execvp(temp_command[0], temp_command);
							wait(NULL); // Waits for child process to finish
						} else if(logic_option == 3) { // 3 (&&&) = executed in parellel in foreground
							// TODO Reap these children
							if (!fork())
								execvp(temp_command[0], temp_command);
						}

						break; // Command executed, exit this loop
					}

					offset++; // Increment offset
				}
				base = base + offset + 1; // Get new base. +1 to traverse over the &,&&, or &&&
				offset = 0; // Reset offset

				free(usr_bin_str);
				usr_bin_str = (char *)malloc(1000); // Reallocates space for usr bin path plus file
				strcpy(usr_bin_str, "/usr/bin/"); // Sets new space to "/usr/bin/"
			}
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}

