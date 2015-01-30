#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>



#define MAXLINE 1000
#define DEBUG 0

//using namespace std;

/* Function declarations and globals */
int parent_pid, status;
char ** tokenize(char*) ;
char ** tokenizeCommands(char*) ;
int execute_command(char** tokens) ;
int ioStreamRedirect(char ** tokens);
int numTokens(char ** tokens);
int sleepVal(char* minute, char* hour);
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
		int status;
		wait(&status);
		printf("Background process killed id: %d with status %d\n", getpid(), status);

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

/*The tokenizer function takes a string of chars(in case of :::) and forms all commands out of it*/
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


int numTokens(char ** tokens){ //counts number of elements in tokens array
	int size = 0;
	for (;tokens[size]!=NULL; size++){
	}
	return size;
}
int ioStreamRedirect(char ** tokens){ //redirects stdin and stdout to files given
	int i;
	for (i = 0; tokens[i]!=NULL; i++){
		//printf("%s\n", tokens[i]);
	}
	int error;
	int size = numTokens(tokens);
	char * arr[MAXLINE];

	int cntl = 0, cntg = 0, cntgg = 0; //count of <. >, and >> for error checking
	for (i=0; i<size; i++){
		if(!strcmp("<", tokens[i]))
			cntl++;
		else if(!strcmp(">>", tokens[i]))
			cntgg++;
		else if(!strcmp(">", tokens[i]))
			cntg++;
	}
	//printf("%d %d %d \n",cntl, cntg, cntgg );
	if(((cntl || cntg || cntgg) && size<3) ||  (cntl + cntg + cntgg >2 || (cntg == cntgg &&  cntg == 1))){
		perror("Wrong syntax"); //printing error for wrong syntax
		return -1;
	}

	if (size >= 3){
		if (size >= 5 && !strcmp("<", tokens[size-4])){ 
			int in = open(tokens[size-3], O_RDONLY);
			dup2(in, 0);
			close(in);
			for(i = 0; i<size-4; i++)
				arr[i] = tokens[i];
			arr[i] = NULL;
		}
		else if (strcmp(">", tokens[size-2]) == 0 || strcmp(">>", tokens[size-2]) == 0 || strcmp("<", tokens[size-2]) == 0){
			for (i = 0; i<size-2; i++){
				arr[i] = tokens[i];
			}
			arr[i] = NULL; //filling null at end of argv
		}
		
		if (strcmp(">", tokens[size-2]) == 0){ //creating new file
			int out = open(tokens[size-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out, 1);
			close(out);
			error = execvp(tokens[0], arr);
		}
		else if (strcmp(">>", tokens[size-2]) == 0){
			int out = open(tokens[size-1], O_APPEND | O_WRONLY); //opening already made file
			if (out < 0) //creating new file
				out = open(tokens[size-1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			dup2(out, 1);
			close(out);
			error = execvp(tokens[0], arr);
		}
		else if (strcmp("<", tokens[size-2]) == 0){ //taking input from file
			int in = open(tokens[size-1], O_RDONLY);
			if (in < 0){
				perror("File doesn't exist/permission denied.");
				return -1;
			}
			dup2(in, 0); //assigning stdin
			close(in);
			error = execvp(tokens[0], arr);
		}
		else error = execvp(tokens[0], tokens);
	}
	else {
		error = execvp(tokens[0], tokens);
	}
	return error; //returning error
}

/*
	Function to find how much time is left for the next job and 
	suspend or sleep the process until then, rather than checking the 
	constraints after every minute
*/

int sleepVal(char* minute, char* hour){
	time_t t = time(0);   // get time now
    struct tm * now = localtime( &t );
    //printf ( "Current local time and date: %s", asctime (now));
    char minuteNow[10], hourNow[10];
    sprintf(minuteNow, "%d", now->tm_min);
    sprintf(hourNow, "%d", now->tm_hour);

	if (!strcmp(hour, "*")){
		if (!strcmp(minute,minuteNow)) return 3600;
		int diff = (atoi(minute) - now->tm_min + 60)%60;
		return diff*60;
	} else if (!strcmp(minute, "*")){
		int diff = ((atoi(hour) - now->tm_hour + 24)%24 + (0 - now->tm_min + 60) + 1440)%1440;
		return diff*60;
	} else{
		if (!strcmp(minute,minuteNow) && !strcmp(hour,hourNow))
			return 86400;
		int diff = ((atoi(hour) - now->tm_hour + 24)%24 + (atoi(minute) - now->tm_min + 60)%60 + 1440)%1440;
		//printf("fkgjf hjkfh gjk %d %d %d %d %d\n", atoi(hour), now->tm_hour, atoi(minute) , now->tm_min, diff);
		return diff*60;
	}
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
	} else if (!strcmp(tokens[numTokens(tokens)-1], "&")){
		tokens[numTokens(tokens)-1] = NULL;
		int pid;
		//runBack = 1;
		if ((pid = fork()) == 0){
			//cntBack = 1;
			/*
			if (sigignore(SIGINT) == SIG_ERR)		// Upon receiving a SIGINT, the signal calls the handler sig_handler
		        printf("\ncan't remove SIGINT\n");
			if (sigignore(SIGQUIT) == SIG_ERR)		// Upon receiving a SIGINT, the signal calls the handler sig_handler
		        printf("\ncan't remove SIGQUIT\n");	*/	    
			execute_command(tokens);
			//signal(SIGINT, SIG_IGN);
			//signal(SIGQUIT, SIG_IGN);
		}
		else {
			//cntBack = 1;
			//signal(SIGCHLD, sig_child_handler);
			
			//back[cntBack++] = pid;
			//printf("Background Process created with pid: %d\n", pid);
		}
		return 0;
	} else if (!strcmp(tokens[0],"cron")){

		int pid;
		//runBack = 1;
		if ((pid = fork()) == 0){	// create a new process for cron in background
			
			fflush(stdin); //flushing the input
			FILE *ifp;

			ifp = fopen(tokens[1], "r"); //opening file for reading
			if (ifp == NULL) {
			  perror("File Not found");
			  exit(-1);
			}
			char c[1000];
			while(fgets(c, 1000, ifp) != NULL ){ //reading line by line
				//printf("Command: %s\n", c);

				int cronPid;	// a new process for every command in the input file
				cronPid = fork();
				if (cronPid == 0){
					char ** newTokens = tokenize(c);
					int x = 0;
					int i;
					if (!strcmp(newTokens[0], "#")) x++;

					time_t t = time(0);   // get time now
				    struct tm * now = localtime( &t );		// Current system time
				    //printf ( "Current local time and date: %s", asctime (now));
				    char minuteNow[10], hourNow[10];
				    sprintf(minuteNow, "%d", now->tm_min);
				    sprintf(hourNow, "%d", now->tm_hour);

				    char newCommand[MAXLINE];
				    strcpy(newCommand,"");
				    for (i = x+2; newTokens[i]!= NULL; i++){
				    	strcat(newCommand, newTokens[i]);
				    	strcat(newCommand, " ");
				    }
				    char** newExec = tokenize(newCommand);

					if (!strcmp(newTokens[x], "*") && !strcmp(newTokens[x], "*")){	// every minute execution
						while(1){
							//printf("%s\n", newExec[0]);
							execute_command(newExec);
							//printf("ksrgfk\n");
							sleep(60);
						}
					} else if (!strcmp(newTokens[x], "*")){
						if (!strcmp(hourNow, newTokens[x+1])){
							for (i = now->tm_min; i < 60; i++){
								execute_command(newExec);
								//printf("ksrgfk\n");
								sleep(60);
							}
						}
						while(1){
							/*
							 sleepval function for computing sleep and not checking at every minute
							*/
							sleep(sleepVal(newTokens[x], newTokens[x+1]));	
							for (i = 0; i < 60; i++){
								execute_command(newExec);
								sleep(60);
							}
						}
					} else{
						while(1){

							/*
							 sleepval function for computing sleep and not checking at every minute
							*/

							sleep(sleepVal(newTokens[x], newTokens[x+1]));
							execute_command(newExec); // execute process
						}
					}

					/* freeing the memory */
					for(i=0;newTokens[i]!=NULL;i++){ 
						free(newTokens[i]);
					}
					free(newTokens);	

					for(i=0;newExec[i]!=NULL;i++){ 
						free(newExec[i]);
					}
					free(newExec);
				}
				else{
					//printf("cron Background Process created with pid: %d\n", cronPid);
				}
			}
			fclose(ifp); //closing file stream


		}
		else {
			//cntBack = 1;
			//signal(SIGCHLD, sig_child_handler);
			
			//back[cntBack++] = pid;
			//printf("Background Process created with pid: %d\n", pid);
			int returnStatus; 
		    //wait(NULL); 
		      
		    waitpid(pid, &returnStatus, 0);  // Parent process waits here for child to terminate.
		    /*
		   	if (WIFEXITED(returnStatus) && WEXITSTATUS(returnStatus) == 0){
		   		return 0;  
		   	}
		   	else{
		   		return -1;
		   	}
		   	*/
		   	
		    if (returnStatus == 0)  // Verify child process terminated without error.  
		    {
		      // printf("The child process terminated normally.\n");  
		      return 0;  
		    }

		    else
		    {
		    	//printf("The child process terminated with an error!.\n");  
		    	return -1;
		    }
		}

		
		//waitpid(-1, status, 0);
		//sleep(2);
		//exit (0) ;
	} else if (!strcmp(tokens[0],"exit")) {
		/* Quit the running process */
		kill(-getpgid(parent_pid), SIGKILL);	// Killing every process
		exit(0);
		//return -1;
	} else if (!strcmp(tokens[0],"cd")) {
		if (chdir(tokens[1]) < 0){
			perror("Directory doesn't exists error ");
			return 1;
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
				execute_command(tokenize(newTokens[i])); //executing commands
				exit(0);
			}
			arr[i] = cid; //storing pids of children
		}
		int status, j;
		for(j=0; j<i; j++){
			waitpid(arr[i], &status, 0); // waiting for all child process to finish
		}
		for(i=0;newTokens[i]!=NULL;i++){ //freeing the memory
			free(newTokens[i]);
		}
		free(newTokens);		
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
					int err = execute_command(tokenize(newTokens[i])); //executing commands sequentially
					if (err < 0) {
						//waitpid(getpid(), SIGKILL);
						return -1;
						//break;
					}
				}
				for(i=0;newTokens[i]!=NULL;i++){ //freeing the memory
					free(newTokens[i]);
				}
				free(newTokens);				
				
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
				for(i=1; tokens[i]!=NULL; i++){ //parsing is being done
					strcat(commands, " ");
					strcat(commands, tokens[i]);
					strcat(commands, " ");
				}
				strcat(commands, ":::\n"); //parsing sequential or commands
			//	printf("Commands:  %s\n", commands);
				newTokens = tokenizeCommands(commands);

				for(i=0; newTokens[i]!=NULL; i++){
					//printf("%s\n", newTokens[i]);
					int err = execute_command(tokenize(newTokens[i]));
					if (err >= 0)
						return -1;
				}
				for(i=0;newTokens[i]!=NULL;i++){ //freeing the memory
					free(newTokens[i]);
				}
				free(newTokens);
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
				  exit(-1);
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
				waitpid(-1, status, 0);
				//sleep(2);
				exit (0) ;
			}
			else {
				/* File Execution */
				/* Print error on failure, exit with error*/

				int pipePresent = 0;

				char command1[MAXLINE], command2[MAXLINE]; 
				strcpy(command1, "");
				strcpy(command2, "");

				int i;
				for(i=0;tokens[i]!=NULL;i++){ //separating commands before and after pipe
					if (!strcmp(tokens[i], "|")){
						pipePresent = 1;
						continue;
					}
					if (pipePresent == 0){
						strcat(command1, tokens[i]);
						strcat(command1, " ");
					}
					else{
						strcat(command2, tokens[i]);
						strcat(command2, " ");
					}
				}


				if (pipePresent == 1){ // execute the piped instructions

					int pipefd[2]; // Pipe File descriptor
					int pipePid;

					char** beforePipe = tokenize(command1);
					char** afterPipe = tokenize(command2);

					pipe(pipefd);  // creating a pipe file descriptor go in pipefd[0] and pipefd[1]

					pipePid = fork();

					if (pipePid == 0) {		// child process handles command after pipe
						dup2(pipefd[1], 1); // input part of pipe
						close(pipefd[0]); 	// close unused part of pipe

						int error = ioStreamRedirect(beforePipe);
						
						
						if (error < 1){ //if error occurs
							perror("Broken Pipe");
							exit(-1);
						}
						waitpid(-1, status, 0);
						//_exit(EXIT_SUCCESS);
						exit(0);
						//return 0;
					}
					else{	// parent process handles command before pipe
						dup2(pipefd[0], 0); // output part of pipe
						close(pipefd[1]);  // close unused part of pipe

						int error = ioStreamRedirect(afterPipe);

						if (error < 1){ //if error occurs
							perror("Broken Pipe");
							exit(-1);
						}
						//wait(NULL);
						waitpid(pipePid, status, 0);
						//exit(EXIT_SUCCESS);
					}
					//waitpid(-1, status, 0);

					for(i=0;afterPipe[i]!=NULL;i++){ //freeing the memory
						free(afterPipe[i]);
					}
					free(afterPipe);

					for(i=0;beforePipe[i]!=NULL;i++){ //freeing the memory
						free(beforePipe[i]);
					}
					free(beforePipe);
					waitpid(-1, status, 0);
					exit(0);			
				}

				else{	// execute non-piped instructions
					int error = ioStreamRedirect(tokens);
					
					if (error < 1){ //if error occurs
						perror("Error in command");
						exit(-1);
					}
					waitpid(-1, status, 0);
					//fflush(stdout); //flushing output
					exit(0);
				}
			}
			return 1;
		}
		else {
			/* Parent Process */
			/* Wait for child process to complete */
		    int returnStatus; 
		    //wait(NULL); 
		      
		    waitpid(pid, &returnStatus, 0);  // Parent process waits here for child to terminate.
		    /*
		   	if (WIFEXITED(returnStatus) && WEXITSTATUS(returnStatus) == 0){
		   		return 0;  
		   	}
		   	else{
		   		return -1;
		   	}
		   	*/
		   	
		    if (returnStatus == 0)  // Verify child process terminated without error.  
		    {
		      // printf("The child process terminated normally.\n");  
		      return 0;  
		    }

		    else
		    {
		    	//printf("The child process terminated with an error!.\n");  
		    	return -1;
		    }
		    
		}
	}
	return 1 ;
}
