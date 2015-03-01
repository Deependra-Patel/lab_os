#include <bits/stdc++.h>
#include <vector>
#include <string>
#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>

// #include <malloc.h>
#define MAXLINE 1000

using namespace std;

//declarations
char ** tokenize(char*);
char ** get_input();



int segment_id, shared_segment_size = 1024;

string to_string(int x){
	string r = "";
	while(x){
		r = char(x%10) + r;
		x/=10;
	}
	return r;
}

void parentHandler (int sig, siginfo_t *siginfo, void *context)
{	
	char* shared_memory = (char*) shmat (segment_id, 0, 0);
	printf("%s\n", shared_memory);
}

void handler (int sig, siginfo_t *siginfo, void *context)
{	
	static vector<string> username;
	cout << "printing username:" << " " << username.size() << " \n";

	char* shared_memory = (char*) shmat (segment_id, 0, 0);
	//printf("%s\n", shared_memory);
	string x(shared_memory);
	cout << x << endl;

	int first = x.find(":");
	string type = x.substr(0, first), user, domain;
	x = x.substr(first+1);
	first = x.find(":");
	user = x.substr(0, first);
	
 	x = x.substr(first+1);
	domain = x;	
	

	if (type == "add_email"){
		int i; 
		for ( i=0; i < username.size(); i++){
			if(user == username[i]){
				//cout<<"Found"<<endl;
				string result = "Child process " + domain + " - Email address already exists.\n";
				strcpy(shared_memory, result.c_str());
				break;
			}
		}
		if(i==username.size()) {
			username.push_back(user);
			string result = "Child process " + domain + " - Email address " + user +"@"+ domain + " added successfully.\n";
			strcpy(shared_memory, result.c_str());
		}
		kill(getppid(), SIGUSR1);
		for ( i=0; i < username.size(); i++)
			cout << i << " 	username:::::  " << username[i] << endl;
	}
	else if (type == "search_email"){
		int i; 
		for ( i=0; i < username.size(); i++){
			if(user == username[i]){
				//cout<<"Found"<<endl;

				string result = "Parent process - found the email address " + user +"@"+ domain + "at " + to_string(i)  + "\n";
				strcpy(shared_memory, result.c_str());
				break;
			}
		}
		if(i==username.size()) {
			string result = "Parent process - could not find the email address " +  user +"@" + domain + "\n";
			strcpy(shared_memory, result.c_str());
		}
		kill(getppid(), SIGUSR1);
	}
	else if (type == "delete_email"){
		int i; 
		for ( i=0; i < username.size(); i++){
			if(user == username[i]){
				//cout<<"Found"<<endl;
				username.erase(username.begin() + i);
				string result = "Child process - child " + domain + " deleted " + user +"@"+ domain + " from position " + to_string(i) + ".\n";
				strcpy(shared_memory, result.c_str());
				break;
			}
		}
		if(i==username.size()) {
	
			string result = "Parent process - child " + domain + "could not find the email address " + user + "\n";
			strcpy(shared_memory, result.c_str());

		}
		kill(getppid(), SIGUSR1);
	}

	else if (type == "delete_domain"){
		cout << "herer fxg deleetet\n";
		int i;
		cout << "Printing usernames of domain: " << endl; 
		for ( i=0; i < username.size(); i++){
				cout << username[i] << endl;
		}
		printf("Done Printing\n");
		for ( i=0; i < username.size(); i++)
			cout << i << " 	username:::::  " << username[i] << endl;

	}

    //printf ("Sending PID: %ld, UID: %ld\n",
    //        (long)siginfo->si_pid, (long)siginfo->si_uid);
}

int main(){
	
	struct sigaction act1;					
    memset (&act1, '\0', sizeof(act1));
    act1.sa_sigaction = &parentHandler;
    act1.sa_flags = SA_SIGINFO;
    if (sigaction(SIGUSR1, &act1, NULL) < 0) {
        perror ("sigaction");
        return 1;
    }

	map<string, int> domainList;
	map<string, int> ::iterator itr, itr1, itr2;

	segment_id = shmget (IPC_PRIVATE, shared_segment_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	/* Attach the shared memory segment. */
	char* shared_memory = (char*) shmat (segment_id, 0, 0);
	printf ("shared memory attached at address %p\n", shared_memory);

	
	for (int u = 0; u < 4; u++){
		char ** tokens = get_input() ;
		
		if (tokens[0] == NULL){
			printf("Invalid Instruction\n");	
		}
		/*
		else if (tokens[0] == "0") {
			cout << "here2\n";
			break;
		}
		*/
		else if (strcmp(tokens[0], "add_email") == 0){
			sprintf(shared_memory, "%s:%s:%s", tokens[0], tokens[1], tokens[2]);
			//printf("472y7842 y74 674 %s\n", shared_memory);
			itr = domainList.find(tokens[2]);
			int pid;
			if (itr == domainList.end()) {
				pid = fork();
				if (pid == 0){ // child process
					
					
				//	printf("dkgbfgbjkfg  %s\n", shared_memory);
					struct sigaction act;
	     
			        memset (&act, '\0', sizeof(act));
			     
			        /* Use the sa_sigaction field because the handles has two additional parameters */
			        act.sa_sigaction = &handler;
			     
			        /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
			        act.sa_flags = SA_SIGINFO;

			        if (sigaction(SIGUSR1, &act, NULL) < 0) {
			            perror ("sigaction");
			            return 1;
			        }
			        //cout << "here1\n";
			        while (1) {
			            sleep (10);
			        }
					_exit(0);
				}
				else{
					domainList.insert(map<string, int >::value_type(tokens[2], pid));
				}
			}
			else{
				pid = itr->second;
			}
			
			sleep(1);
			kill(pid, SIGUSR1);

			printf("add_email\n");
		}


		else if (strcmp(tokens[0], "search_email") == 0){
			itr = domainList.find(tokens[2]);
			if (itr == domainList.end()){
				printf("Parent process - Domain does not exist.\n");
			}
			else{
				sprintf(shared_memory, "%s:%s:%s", tokens[0], tokens[1], tokens[2]);
				int pid = itr->second;
				sleep(1);
				kill(pid, SIGUSR1);
			}
			printf("s_email\n");
		}



		else if (strcmp(tokens[0], "delete_email") == 0){
			itr = domainList.find(tokens[2]);
			if (itr == domainList.end()){
				printf("Parent process - Domain does not exist.\n");
			}
			else{
				sprintf(shared_memory, "%s:%s:%s", tokens[0], tokens[1], tokens[2]);
				int pid = itr->second;
				sleep(1);
				kill(pid, SIGUSR1);
			}
			printf("d_email\n");
		}

		else if (strcmp(tokens[0], "delete_domain") == 0){
			itr = domainList.find(tokens[1]);
			if (itr == domainList.end()){
				printf("Parent process - Domain does not exist.\n");
			}
			else{
				sprintf(shared_memory, "%s:%s:%s", tokens[0], tokens[0], tokens[1]);
				//printf("%s\n", shared_memory);
				int pid = itr->second;
				sleep(1);
				kill(pid, SIGUSR1);
				kill(pid, SIGKILL);
				cout << "Parent process - Domain " << tokens[1] << "with PID - " << pid << " deleted.\n";
				domainList.erase(itr);
			}
			printf("dd\n");
		}


		else if (strcmp(tokens[0], "send_email") == 0){

			itr1 = domainList.find(tokens[2]);
			itr2 = domainList.find(tokens[4]);
			if(itr1!=domainList.end() && itr2!=domainList.end()){
				sprintf(shared_memory, "%s:%s:%s", tokens[0], tokens[1], tokens[2]);
				//str = tokens[0] + tokens[1] + tokens[2] + string(*itr2) + tokens[3];
				//strcpy(shared_memory, str);
				//cout<<"Parent: Sending main process"<<endl;
				//kill(*itr1, SIGUSR1);
			}

			else cout<<"Domain Name of sender/receiver doesn't exist";
			printf("se\n");
		}

		else{
			printf("Invalid Instruction\n");	
		}
		// Freeing the allocated memory
		int i;
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
		sleep(1);
	}
	return 0;
}


/*the tokenizer function takes a string of chars and forms tokens out of it*/
char ** tokenize(char* input){
	int i;
	int doubleQuotes = 0;
	
	char *token = (char *)malloc(1000*sizeof(char));
	int tokenIndex = 0;

	char **tokens;
	tokens = (char **) malloc(MAXLINE*sizeof(char*));

	int tokenNo = 0;
	
	for(i =0; i < strlen(input); i++){
		char readChar = input[i];
		if (readChar == '@'  || readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAXLINE*sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else{
			token[tokenIndex++] = readChar;
		}
	}
	
	tokens[tokenNo] = NULL ;
	return tokens;
}

char ** get_input() {
	FILE* stream = stdin;
	char input[MAXLINE];
	fflush(stdin) ;

	char *in = fgets(input, MAXLINE, stream); // taking input ;
	if (in == NULL){
		return NULL ;
	}
	// Calling the tokenizer function on the input line    
	char **tokens = tokenize(input);	
	/*
	// Comment to NOT print tokens
	int i ;
	for(i=0;tokens[i]!=NULL;i++){
		printf("%s\n", tokens[i]);
	}   
	*/
	return tokens ;
}

