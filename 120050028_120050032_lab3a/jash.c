#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
//#include <iostream>
//#include <fstream>
//#include <string>


#define MAXLINE 1000
#define DEBUG 0

//using namespace std;

/* Function declarations and globals */
int parent_pid, error;
char ** tokenize(char*) ;
int execute_command(char** tokens) ;

/**
The signal handler function:
This function handles signals sent to all the processes.
The child processes will also inherit this handler from the parent.
Upon receiving SIGQUIT or SIGINT, all the child processes are killed
*/

void sig_handler(int signo)
{
	if (getpid() != parent_pid){	// Checking if the process is a child process 
	    if (signo == SIGINT)
	    {
	        printf("received SIGINT\n");
	    }
	    else if (signo == SIGQUIT){
	        printf("received SIGQUIT\n");
	    }
	    kill(getpid(), SIGKILL); // Killing the process
	}
}

int main(int argc, char** argv){

	
	parent_pid = getpid();
	
	/* Set (and define) appropriate signal handlers */
	/* Exact signals and handlers will depend on your implementation */
	// signal(SIGINT, quit);
	// signal(SIGTERM, quit);

    if (signal(SIGINT, sig_handler) == SIG_ERR)		// Upon receiving a SIGINT, the signal calls the handler sig_handler
        printf("\ncan't catch SIGINT\n");
    if (signal(SIGQUIT, sig_handler) == SIG_ERR)	// Upon receiving a SIGQUIT, the signal calls the handler sig_handler
        printf("\ncan't catch SIGQUIT\n");
    //if (signal(SIGSTOP, sig_handler) == SIG_ERR)
      //  printf("\ncan't catch SIGSTOP\n");
    
    // A wait so that we can easily issue a signal to this process
    sleep(1);


	char input[MAXLINE];
	char** tokens;
	
	while(1) { 
		printf("$ "); // The prompt
		fflush(stdin);

		char *in = fgets(input, MAXLINE, stdin); // Taking input one line at a time
		//Checking for EOF
		if (in == NULL){
			if (DEBUG) printf("jash: EOF found. Program will exit.\n");
			break ;
		}

		// Calling the tokenizer function on the input line    
		tokens = tokenize(input);	
		// Executing the command parsed by the tokenizer
		execute_command(tokens) ; 
		
		// Freeing the allocated memory	
		int i ;
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	
	/* Kill all Children Processes and Quit Parent Process */
	return 0 ;
}

/*The tokenizer function takes a string of chars and forms tokens out of it*/
char ** tokenize(char* input){
	int i, doubleQuotes = 0, tokenIndex = 0, tokenNo = 0 ;
	char *token = (char *)malloc(MAXLINE*sizeof(char));
	char **tokens;

	tokens = (char **) malloc(MAXLINE*sizeof(char*));

	for(i =0; i < strlen(input); i++){
		char readChar = input[i];

		if (readChar == '"'){
			doubleQuotes = (doubleQuotes + 1) % 2;
			if (doubleQuotes == 0){
				token[tokenIndex] = '\0';
				if (tokenIndex != 0){
					tokens[tokenNo] = (char*)malloc(MAXLINE*sizeof(char));
					strcpy(tokens[tokenNo++], token);
					tokenIndex = 0; 
				}
			}
		} else if (doubleQuotes == 1){
			token[tokenIndex++] = readChar;
		} else if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAXLINE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		} else {
			token[tokenIndex++] = readChar;
		}
	}

	if (doubleQuotes == 1){
		token[tokenIndex] = '\0';
		if (tokenIndex != 0){
			tokens[tokenNo] = (char*)malloc(MAXLINE*sizeof(char));
			strcpy(tokens[tokenNo++], token);
		}
	}

	tokens[tokenNo] = NULL ;
	return tokens;
}

int execute_command(char** tokens) {
	/* 
	Takes the tokens from the parser oand based on the type of command 
	and proceeds to perform the necessary actions. 
	Returns 0 on success, -1 on failure. 
	*/
	if (tokens == NULL) {
		return -1 ; 				// Null Command
	} else if (tokens[0] == NULL) {
		return 0 ;					// Empty Command
	} else if (!strcmp(tokens[0],"exit")) {
		/* Quit the running process */
		return 0 ;
	} else if (!strcmp(tokens[0],"cd")) {
		//printf("%s\n", tokens[1]);
		if (chdir(tokens[1]) < 0){
			perror("Directory doesn't exists error ");
			return -1;
		}
		else{
			char cwd[1024];
			if (getcwd(cwd, sizeof(cwd)) != NULL)
				fprintf(stdout, "Current working dir: %s\n", cwd);
		}
		/* Change Directory, or print error on failure */
		return 0 ;
	} else if (!strcmp(tokens[0],"parallel")) {
		/* Analyze the command to get the jobs */
		/* Run jobs in parallel, or print error on failure */
		return 0 ;
	} else if (!strcmp(tokens[0],"sequential")) {
		/* Analyze the command to get the jobs */
		/* Run jobs sequentially, print error on failure */
		/* Stop on failure or if all jobs are completed */
		return 0 ;					// Return value accordingly
		} else {
		/* Either file is to be executed or batch file to be run */
		/* Child process creation (needed for both cases) */
		int pid = fork() ;
		if (pid == 0) {
			if (!strcmp(tokens[0],"run")) {
				/* Locate file and run commands */
				/* May require recursive call to execute_command */
				/* Exit child with or without error */

				//cout<<"inside run"<<endl;
				fflush(stdin);

				FILE *ifp;

				ifp = fopen(tokens[1], "r");

				if (ifp == NULL) {
				  perror("File Not found");
				  exit(1);
				}
				char c[1000];
				while(fgets(c, 1000, ifp) != NULL ){
					printf("Command: %s\n", c);
					char ** newTokens = tokenize(c);
					execute_command(newTokens);
					int i ;
					for(i=0;newTokens[i]!=NULL;i++){
						free(newTokens[i]);
					}
					free(newTokens);	
				}
				fclose(ifp);

				sleep(10);
				exit (0) ;
			}
			else {
				/* File Execution */
				/* Print error on failure, exit with error*/
				//cout<<"File execution"<<endl;
				int error = execvp(tokens[0], tokens);
				if (error < 1)
					perror("Error occured ");
				fflush(stdout);
				exit(0) ;
			}
		}
		else {
			/* Parent Process */
			/* Wait for child process to complete */
		}
	}
	return 1 ;
}
