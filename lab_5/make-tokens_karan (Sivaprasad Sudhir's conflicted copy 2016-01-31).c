#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char* server_ip;
char* server_port;
int server_set;

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
		} 
		else {
			token[tokenIndex++] = readChar;
		}
	}

free(token);
tokens[tokenNo] = NULL ;
return tokens;
}


void  main(void)
{
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;
    server_set=0;

    while (1) {           
      
    	printf("Hello>");     
    	bzero(line, MAX_INPUT_SIZE);
    	gets(line);           
    	//printf("Got command %s\n", line);
    	line[strlen(line)] = '\n'; //terminate with new line
    	tokens = tokenize(line);
   
    	//do whatever you want with the commands, here we just print them

    	for(i=0;tokens[i]!=NULL;i++){
			//printf("found token %s\n", tokens[i]);
    	}
		int no_tokens=i;
		
		if(i==0) {
			continue;
		}

// cd
		if(strcmp(tokens[0],"cd")==0) {
			if(no_tokens!=2) {
				printf("Format cd <directory> \n");
			}
			else {
				int err = chdir(tokens[1]);
				if(err==-1) {
					printf("cd: Error in changing directory\n");
				}
			}
		}
		
//ls 
		if(strcmp(tokens[0],"ls")==0)
		{	if(no_tokens!=1) {
				printf("error");
			}
			else {
				int pid=fork();
				if(pid<0) printf("%s\n","Error");
				if(pid==0) {	
					execl("/bin/ls", "ls", NULL);
					exit(0);
				}
				waitpid(pid,0,NULL);
			}
		}
		
//cat 
		if(strcmp(tokens[0],"cat")==0)
		{	if(no_tokens<2) {
				printf("error:Expected input : cat list_files \n ");
			}
			else {
				int pid=fork();
				if(pid<0) printf("%s\n","Error");
				if(pid==0) {	
					
					execv("/bin/cat",tokens);
					exit(0);
				}
				waitpid(pid,0,NULL);
			}
		}

//echo 
		if(strcmp(tokens[0],"echo")==0)
		{	if(no_tokens<2) {
				printf("error:Expected input : echo list_files \n ");
			}
			else {
				int pid=fork();
				if(pid<0) printf("%s\n","Error");
				if(pid==0) {	
					
					execv("/bin/echo",tokens);
					exit(0);
				}
				waitpid(pid,0,NULL);
			}
		}

//server
		if(strcmp(tokens[0],"server")==0)
		{	
			if(no_tokens!=3) {
				printf("error:Expected input : server server_IP server_port \n ");
			}
			else {
				free(server_ip);
				free(server_port);
				server_ip=malloc(strlen(tokens[1]));
				server_port=malloc(strlen(tokens[2]));
				strcpy(server_ip, tokens[1]);
				strcpy(server_port, tokens[2]);
				server_set=1;
			}
		}
		
//getfl filename
		if(strcmp(tokens[0],"getfl")==0)
		{	if(no_tokens!=2) {
				printf("error:Expected input : getfl filename \n ");
			}
			else {
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else {
					int pid=fork();
					if(pid<0) printf("%s\n","Error");
					if(pid==0) {	
						execl("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL);
						exit(0);
					}
					waitpid(pid,0,NULL);
				}
			}
		}

//getsq file1 file2
		if(strcmp(tokens[0],"getsq")==0)
		{	if(no_tokens<2) {
				printf("error:Expected input :  getsq file1 file2 ... \n ");
			}
			else {
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else {
					int j=0; 
					for(j=0;j<no_tokens-1;j++) {
						int pid=fork();
						if(pid<0) printf("%s\n","Error");
						if(pid==0) {	
							execl("./get-one-file-sig", "./get-one-file-sig", tokens[j+1], server_ip, server_port, "nodisplay", NULL);
							exit(0);
						}
						waitpid(pid,0,NULL);
					}
				}
			}
		}

//getpl file1 file2
		if(strcmp(tokens[0],"getpl")==0)
		{	if(no_tokens<2) {
				printf("error:Expected input :  getpl file1 file2 ... \n ");
			}
			else {
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else {
					int j=0;
					int * pids = malloc(sizeof(int)*(no_tokens-1)); 
					for(j=0;j<no_tokens-1;j++) {
						int pid=fork();
						pids[j]=pid;
						if(pid<0) printf("%s\n","Error");
						if(pid==0) {	
							execl("./get-one-file-sig", "./get-one-file-sig", tokens[j+1], server_ip, server_port, "nodisplay", NULL);
							exit(0);
						}
					}
					for(j=0;j<no_tokens-1;j++) {
						waitpid(pids[j],0,NULL);
					}
				}
			}
		}



    	// Freeing the allocated memory	
    	for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
    	}
    	free(tokens);
    }
     

}

                
