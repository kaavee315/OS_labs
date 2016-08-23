/* A simple server in the internet domain using TCP
The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void sig_chld(int signo)
{	pid_t    pid;
	int      stat;
	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
    return;
}


int main(int argc, char *argv[])
{	
	/*int pid_strt=fork();
	if(pid_strt<0)
	{	perror("ERROR on fork_0");
        exit(1);
    }
    else if(pid_strt>0)
    {	while(1)waitpid(-1.0,WNOHANG);
	}*/
    int sockfd, newsockfd, portno, clilen;	
    char buffer[1024];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    /* create socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    error("ERROR opening socket");
    /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    /* bind socket to this port number on this machine */
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
    sizeof(serv_addr)) < 0)
    error("ERROR on binding");
    /* listen for incoming connection requests */
    listen(sockfd,5);
	clilen = sizeof(cli_addr);
    int pid;
    int count=0;
    //signal (SIGCHLD, sig_chld); /* must call waitpid() */
    while(1) {
		/* accept a new request, create a newsockfd */
		//while(waitpid(-1,0,WNOHANG)>0);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        {	if (errno == EINTR)
            continue;
			error("ERROR on accept");
		}
        pid=fork();
        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }
        if (pid == 0) {
            /* This is the client process */
            /*close main socket in child process*/
            close(sockfd);
            {	
				//buffer to read the file sent by client
                bzero(buffer,1024);
                n = read(newsockfd,buffer,1023);
                if (n < 0) error("ERROR reading from socket");
                printf("Here is the message: %s \n",buffer);
                char* fname;
                fname = (char* )malloc(strlen(buffer)-4);
                int i=4;
                while(1)
                {
                    if(buffer[i]=='.' && buffer[i+1]=='t' && buffer[i+2]=='x' && buffer[i+3]=='t')break;
                    i++;
                }
                fname = (char* )malloc(i);
                int j;
                for(j=4;j<i+4;j++)
                {
                    fname[j-4]=buffer[j];
                }
                //printf("Here is the fname: %s yoyo\n",fname);
                // file to be written
                FILE *fp = fopen(fname,"rb");
                if(fp==NULL)
                {
                    printf("File open error");
                    return 1;
                }
                //fscanf(f,"%s",buffer);
                while(1)
                {
                    /* First read file in chunks of 256 bytes */
                    unsigned char buff2[1024]={0};
                    unsigned char buff[512]={0};
                    int nread = fread(buff,1,512,fp);
                    //printf("Bytes read %d \n", nread);
                    if(nread<512)
                    {   if(nread>0)
                        {   int k=0;
                            for(k=0;k<512;k++)
                            { buff2[k]=buff[k];
                            }
                        //printf("Sending \n");
                        write(newsockfd, buff2, nread);
                        }
                        if (feof(fp))
                        //printf("End of file\n");
                        if (ferror(fp))
                        printf("Error reading\n");
                        unsigned char newbuff[1024]={'\0'};
                        write(newsockfd, newbuff,0);
                        break;
                    }
                    int k=0;
                    for(k=0;k<512;k++)
                    { buff2[k]=buff[k];
                    }
                    int nread2 = fread(buff,1,512,fp);
                    //printf("Bytes read %d \n", nread2);
                    /* If read was success, send data. */
                    if(nread2 > 0)
                    {   int k=0;
                        for(k=512;k<1024;k++)
                        { buff2[k]=buff[k-512];
                        }
                    }
                    //printf("Sending \n");
                    write(newsockfd, buff2, nread + nread2);
                   
                    if (nread2 < 512)
                    {
                        if (feof(fp))
                        printf("End of file\n");
                        if (ferror(fp))
                        printf("Error reading\n");
                        unsigned char newbuff[1024]={'\0'};
                        write(newsockfd, newbuff,0);
                        break;
                    }
                }
                //n = write(newsockfd,buffer,255);
                // if (n < 0) error("ERROR writing to socket");
            
            }
            close(newsockfd); 
            exit(0);
        }
        else {
			/*close new sockets in parent process*/
            close(newsockfd);
        }
    }
    /*close main socket*/
    close(sockfd);
    return 0;
}
