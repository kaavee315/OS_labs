#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define RD_WR 0666

char* server_ip;
char* server_port;
int server_set;
volatile sig_atomic_t keep_going = 1;
int curr_back=-1;
int no_bgp=0;

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

//All the foreground processes are in the same process group as the client shell
//They receive SIGINT with client shell and handles it accordingly
//The background processes are in a different group
//The client shells sets keep_going to 0 to avoid further forks in the same command
void handler(int signo)
{	
	keep_going = 0;
    return;
}

void  main(void)
{
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;
    server_set=0;
    signal (SIGINT, handler); /* must call waitpid() */
	int arg_set=0;
	int no_tokens=0;

    while (1) {
      	if(arg_set==0) {
      		printf("Hello>");     
	    	bzero(line, MAX_INPUT_SIZE);
	    	gets(line);           
	    	line[strlen(line)] = '\n'; //terminate with new line
	    	tokens = tokenize(line);
	    	for(i=0;tokens[i]!=NULL;i++){
	    	}
			no_tokens=i;
			
			if(i==0) {
				continue;
			}
      	}

// cd
		if(strcmp(tokens[0],"cd")==0) {
			if(no_tokens!=2) {
				printf("Format cd <directory> \n");
			}
			else {
				int err = chdir(tokens[1]);
				if(err<0) {
					perror("Error ");
				}
			}

		}
		
//server
		else if(strcmp(tokens[0],"server")==0)
		{	
			if(no_tokens!=3) {
				printf("error:Expected input : server server_IP server_port \n");
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


// getfl filename
//Supports | and >
		
		else if(strcmp(tokens[0],"getfl")==0)
		{	
			if(no_tokens<2)	{	
				printf("Format : getfl filename\n");
			}
			else if(no_tokens==2) {
					if(server_set!=1) {
						printf("No server details given\n");
					}
					else {
						int pid=fork();
						if(pid<0) perror("Error ");
						if(pid==0) {	
							if(execl("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL)<0)
								perror("Error ");
							exit(0);
						}
						waitpid(pid,0,NULL);
					}
			}
			else if((strcmp(tokens[2],">")==0 && no_tokens==4)){
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else {
					//close stdout and redirect it to the fd corresponding to the file
					int pid=fork();
					if(pid<0) perror("Error ");
					if(pid==0) {
						int fd;
   						if(fd = creat( tokens[3], RD_WR )<0)perror("Error ");
						if(dup2(fd,1)<0)perror("Error ");
						if(execl("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL)<0)
							perror("Error ");
						exit(0);
					}
					waitpid(pid,0,NULL);
					
				}
			}
			else if(strcmp(tokens[2],"|")!=0 ) {
				printf("error:Expected input : getfl filename or getfl filename > output \n");
			}
			else  {
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else 
				{	
					//create a pipe
					//close stdout in instrchild1 and dup the write end to it
					//close stdin in child2 and dup read end to it
					//close both the fds in botht children and parent
					int fd[2];
					if(pipe(fd)<0)
						printf("Pipe could not be opened\n");
					int pid1=fork();
					if(pid1<0) 
						printf("%s\n","Error in forking");
					if(pid1==0) {
						if(dup2(fd[1],1)<0)
							perror("Error ");;
						close(fd[0]);
						close(fd[1]);
						if(execl("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL)<0)
							perror("Error ");
						exit(0);
					}

					int pid2=fork();
					if(pid2<0) perror("Error ");
					if(pid2==0) {
						if(dup2(fd[0],0)<0)
							perror("Error ");
						close(fd[0]);
						close(fd[1]);
						arg_set=1;
						int i=0;
						char **temps = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
						for(i=0;i<no_tokens-3;i++)
						{	temps[i]=(char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
							strcpy(temps[i],tokens[i+3]);
						}	
						for(i=0;i<no_tokens;i++)
						{	free(tokens[i]);
						}
						free(tokens);
						no_tokens=no_tokens-3;
						tokens=temps;
						tokens[no_tokens]=NULL;
						continue;

					}
					close(fd[0]);
					close(fd[1]);
					waitpid(pid2,0,NULL);
					waitpid(pid1,0,NULL);
				}
			}
		}
		

//getsq file1 file2
//fork a process
//Wait for it to complete
//Then fork next chils and continue the same
		else if(strcmp(tokens[0],"getsq")==0)
		{	if(no_tokens<2) {
				printf("error:Expected input :  getsq file1 file2 ... \n");
			}
			else {
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else {
					int j=0; 
					for(j=0; (j<no_tokens-1) && keep_going;j++) {
						int pid=fork();
						if(pid<0) perror("Error ");
						if(pid==0) {
							if(execl("./get-one-file-sig", "./get-one-file-sig", tokens[j+1], server_ip, server_port, "nodisplay", NULL)<0)
								perror("Error ");
							exit(0);
						}
						waitpid(pid,0,NULL);
					}
					keep_going=1;
				}
			}
		}

//getpl file1 file2
//Fork all the children
//Wait for all of them afterwards
		else if(strcmp(tokens[0],"getpl")==0)
		{	if(no_tokens<2) {
				printf("error:Expected input :  getpl file1 file2 ... \n");
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
						if(pid<0) perror("Error ");
						if(pid==0) {	
							if(execl("./get-one-file-sig", "./get-one-file-sig", tokens[j+1], server_ip, server_port, "nodisplay", NULL)<0)
								perror("Error ");
							exit(0);
						}
					}
					for(j=0;j<no_tokens-1;j++) {
						waitpid(pids[j],0,NULL);
					}
				}
			}
		}

//getbg filename
//Background processes are kept in a different process group
//which is the pg corresponding to the new background process or the existing group
		else if(strcmp(tokens[0],"getbg")==0)
		{	if(no_tokens!=2) {
				printf("error:Expected input : getbg filename \n");
			}
			else {
				if(server_set!=1) {
					printf("No server details given\n");
				}
				else {
					int pid=fork();
					if(pid<0) printf("%s\n","Error in forking");
					if(pid==0) {	
						if(execl("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "nodisplay", NULL)<0)
							perror("Error ");
						exit(0);
					}
					if(curr_back<0)
					{	curr_back=pid;
						if(setpgid(pid,0)<0) {
							perror("Error ");
						}
					}
					else if(setpgid(pid,curr_back)<0) {
						curr_back=pid;
						if(setpgid(pid,0)<0) {
							perror("Error ");
						}
					}
					no_bgp++;
				}
			}
		}

//exit 
//kill the background process group, reap them and then exit
		else if(strcmp(tokens[0],"exit")==0)
		{	
			kill(-curr_back,SIGINT);
			int i=0;
			for(i=0;i<no_bgp;i++)
			{	waitpid(-curr_back,0,0);
			}
			exit(0);
		}

//others 
//Check in /bin and /usr/bin for the executable
//if not present, print error
		else {
			char temp[80];
			char temp2[80];
			sprintf(temp,"/bin/%s",tokens[0]);
			if(access(temp, F_OK)==-1) {
				sprintf(temp2,"/usr/bin/%s",tokens[0]);
				if(access(temp2, F_OK)==-1) {
					printf("Command not Found\n");
				}
				else 
				{	int pid=fork();
					if(pid<0) perror("Error ");
					if(pid==0) {	
						if(execv(temp2,tokens)<0)perror("Error ");
						exit(0);
					}
					waitpid(pid,0,NULL);
				}
			}
			else 
			{	int pid=fork();
				if(pid<0) perror("Error ");
				if(pid==0) {	
					if(execv(temp,tokens)<0)perror("Error ");
					exit(0);
				}
				waitpid(pid,0,NULL);
			}
		}

//Reap off background process
		int a;
		int status;
		while(a=waitpid(-1,&status,WNOHANG)>0)
		{	
			no_bgp--;
			printf("Background process reaped off with status %d \n",status);
		}
		keep_going=1;

// Freeing the allocated memory	
    	for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
    	}
    	free(tokens);
    	if(arg_set==1) exit(0);
    }
     
}

                
