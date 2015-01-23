#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>



#define MAXLINE 1000
#define DEBUG 0

//using namespace std;

/* Function declarations and globals */
int parent_pid, error;
char ** tokenize(char*) ;
char ** tokenizeCommands(char*) ;
int execute_command(char** tokens) ;
int isParallel;
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

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
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

	tokens[tokenNo] = NULL ;
	return tokens;
}

/*The tokenizer function takes a string of chars and forms tokens out of it*/
char ** tokenizeCommands(char* input){
	int i, tokenIndex = 0, tokenNo = 0 ;
	char *token = (char *)malloc(MAXLINE*sizeof(char));
	char **tokens;
	char **newTokens;
	newTokens = tokenize(input);

	tokens = (char **) malloc(MAXLINE*sizeof(char*));

	char command[1000];
	strcpy(command, "");
	for(i =0; newTokens[i]!= NULL; i++){
		//printf("%i\n", i);
		//printf("%s\n", newTokens[i]);
		if (strcmp(newTokens[i], ":::")){
			strcat(command, newTokens[i]);
			strcat(command, " ");
		}
		else{
			tokens[tokenNo] = (char*)malloc(MAXLINE*sizeof(char));
			strcpy(tokens[tokenNo++], command);
			//printf("%s\n", command);
			strcpy(command, "");
		}
	}

	tokens[tokenNo] = NULL ;
	return tokens;
}


int execute_command(char** tokens) {
	// int i;
	// printf("Printing tokens\n");
	// for(i = 0; tokens[i]!=NULL; i++)
	// 	printf("%s\n", tokens[i]);
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
		kill(parent_pid, SIGKILL);
		return 0 ;
	} else if (!strcmp(tokens[0],"cd")) {
		if (chdir(tokens[1]) < 0){
			perror("Directory doesn't exists error ");
			return 1;
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
		char commands[1000];
		char** newTokens;
		strcpy(commands, "");
		
		int i ;
		for(i=1; tokens[i]!=NULL; i++){
			strcat(commands, " ");
			strcat(commands, tokens[i]);
			strcat(commands, " ");
		}
		strcat(commands, ":::\n");
	//	printf("Commands:  %s\n", commands);
		newTokens = tokenizeCommands(commands);
		int arr[100];
		for(i=0; newTokens[i]!=NULL; i++){
			//printf("%s\n", newTokens[i]);
			int cid;
			if ((cid = fork()) == 0){
				execute_command(tokenize(newTokens[i]));
				exit(0);
			}
			arr[i] = cid;
		}
		int status, j;
		for(j=0; j<i; j++){
			waitpid(arr[i], &status, 0);
		}
		return 0 ;
	} else if (!strcmp(tokens[0],"sequential")) {
			/* Analyze the command to get the jobs */
			/* Run jobs sequentially, print error on failure */
			/* Stop on failure or if all jobs are completed */
				char commands[1000];
				char** newTokens;
				strcpy(commands, "");
				
				int i ;
				for(i=1; tokens[i]!=NULL; i++){
					strcat(commands, " ");
					strcat(commands, tokens[i]);
					strcat(commands, " ");
				}
				strcat(commands, ":::\n");
			//	printf("Commands:  %s\n", commands);
				newTokens = tokenizeCommands(commands);

				for(i=0; newTokens[i]!=NULL; i++){
					//printf("%s\n", newTokens[i]);
					int err = execute_command(tokenize(newTokens[i]));
					if (err < 0)
						break;
				}	
				
			return 0 ;					// Return value accordingly
		}
		else if (!strcmp(tokens[0],"sequential_or")) {
			/* Analyze the command to get the jobs */
			/* Run jobs sequentially, print error on failure */
			/* Stop on failure or if all jobs are completed */
				char commands[1000];
				char** newTokens;
				strcpy(commands, "");
				
				int i ;
				for(i=1; tokens[i]!=NULL; i++){
					strcat(commands, " ");
					strcat(commands, tokens[i]);
					strcat(commands, " ");
				}
				strcat(commands, ":::\n");
			//	printf("Commands:  %s\n", commands);
				newTokens = tokenizeCommands(commands);

				for(i=0; newTokens[i]!=NULL; i++){
					//printf("%s\n", newTokens[i]);
					int err = execute_command(tokenize(newTokens[i]));
					if (err >= 0)
						return -1;
				}	
			return 0 ;					// Return value accordingly
		}

		else {
		/* Either file is to be executed or batch file to be run */
		/* Child process creation (needed for both cases) */
		int pid = fork() ;
		if (pid == 0) {
			if (!strcmp(tokens[0],"run")) {
				/* Locate file and run commands */
				/* May require recursive call to execute_command */
				/* Exit child with or without error */
				fflush(stdin); //flushing the input
				FILE *ifp;

				ifp = fopen(tokens[1], "r"); //opening file for reading
				if (ifp == NULL) {
				  perror("File Not found");
				  exit(1);
				}
				char c[1000];
				while(fgets(c, 1000, ifp) != NULL ){ //reading line by line
					//printf("Command: %s\n", c);
					char ** newTokens = tokenize(c);
					execute_command(newTokens);
					int i ;
					for(i=0;newTokens[i]!=NULL;i++){ //freeing the memory
						free(newTokens[i]);
					}
					free(newTokens);	
				}
				fclose(ifp); //closing file stream
				waitpid(-1, error, 0);
				//sleep(2);
				exit (0) ;
			}
			else {
				/* File Execution */
				/* Print error on failure, exit with error*/
				int error = execvp(tokens[0], tokens);
				
				if (error < 1){ //if error occurs
					perror("Error occured ");
					return -1;
				}
				wait();
				fflush(stdout); //flushing output
				exit(0) ;
			}
			return 1;
		}
		else {
			/* Parent Process */
			/* Wait for child process to complete */
		    int returnStatus;    
		    waitpid(pid, &returnStatus, 0);  // Parent process waits here for child to terminate.

		    if (returnStatus == 0)  // Verify child process terminated without error.  
		    {
		      // printf("The child process terminated normally.\n");  
		      return 0;  
		    }

		    if (returnStatus == 1)      
		    {
		       //printf("The child process terminated with an error!.\n");  
		    	return -1;
		    }
		}
	}
	return 1 ;
}
