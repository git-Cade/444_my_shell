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


		// Print out all tokens
		// TODO: Check if they are &, &&, or &&&

		// 0 = default fork, exec, wait
		// 1 (&) = fork, exec (single process only)
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

		//TODO Insert logic_option dependent ...logic
		int base = 0;
		int offset = 0;
		while(tokens[base]) { 
			while(tokens[base + offset]) {
				printf("%s\n", tokens[base + offset]);
				offset++;
			}
			base = base + offset + 1;
			offset = 0; // Reset offset
			printf("\n");
		}


		//				if(*tokens[base + offset] == '&')
		//			printf("Forking and starting %s in the background\n", tokens[base + offset]);
		//		else if(*tokens[base + offset] == '&&')
		//			printf("Running sequentially\n");
		//		else if(*tokens[base + offset] == '&&&')
		//			printf("Running in parallel\n");


		//for(i=0;i < sizeof(tokens)/sizeof(tokens[0]); i++) {
		//	printf("%s\n", tokens[i]);
		//}
		//char *end_tokens = tokens[sizeof(tokens)/4];
		//printf("%s\n", tokens[0]);
		//printf("%s\n", tokens[1]);
		//char *end_tokens = tokens[sizeof(tokens)/sizeof(tokens[0])];
		//if(end_tokens)
		//	printf("%s\n", end_tokens);

		char* usr_bin_str = malloc(1000);
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
		} else if (access(strcat(usr_bin_str, tokens[0]), F_OK)) { // If command doesn't exist
			printf("Command not found\n");
		//} else if (strlen(line) > 1) { // Any other basic command
		} else {
			int ppid = fork();
			if (!ppid) {
				execvp(tokens[0], tokens);  
			} else {
				wait(NULL); // Waits for child process to finish
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

