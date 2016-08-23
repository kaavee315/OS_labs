#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <signal.h>

long long bytecount;
char recvBuff[1024];
int flag=0;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void handler(int signo)
{   
    if(flag)bytecount=bytecount+strlen(recvBuff);
    printf("Received SIGINT; downloaded %lld bytes so far.\n",bytecount);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    bytecount=0;

    signal (SIGINT, handler); 

    struct sockaddr_in serv_addr;
    struct hostent *server;
    int bytesReceived = 0;
    char buffer[256];
    if (argc < 5) {
       fprintf(stderr,"usage %s file hostname port display/nodisplay\n", argv[0]);
       exit(0);
    }

    /* create socket, get sockfd handle */

    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* fill in server address in sockaddr_in datastructure */

    server = gethostbyname(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    /* connect to server */

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    /* ask user for input */
    
    //printf("Please enter the message: ");
    bzero(buffer,256);
    

    /* send user message to server */
    
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    buffer[0]='g';
    buffer[1]='e';
    buffer[2]='t';
    buffer[3]=' ';
    int alp=4;
    for(alp=0;alp<strlen(argv[1]);alp++)
    {   buffer[alp+4]=argv[1][alp];
    }
    
    n = write(sockfd,buffer,strlen(buffer));
    /* read reply from server */

    /*FILE *fp;
    fp = fopen("ran2", "ab"); 
    if(NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }*/
    
            bzero(recvBuff,1024);
    
     while((bytesReceived = read(sockfd, recvBuff, 1024)) > 0)
        {   
            bytecount=bytecount+bytesReceived;
            flag=1;
            //fp = fopen(filename, "ab"); 
            // bytesReceived = read(sockfd, recvBuff, 1023);
            // printf("Bytes received %d in ",bytesReceived);  
            // printf("Thread no : %d ",threadnum);
            // printf("file no : %d\n",fileno );    

            // recvBuff[n] = 0;
            // printf("Buffer received : %s\n",recvBuff);
            //fwrite(recvBuff,1,bytesReceived,fp);
            //fclose(fp);
            if(argv[4][0]=='d')
            {   printf("%s",recvBuff);
            }
            int j,count=0;
            for(j=0;j<1024;j++)
            {   if(recvBuff[j]=='\0')count++;
            }
            if(count==1024){
                break;
            }
            bzero(recvBuff,1024);
            flag=0;
            
            
        }

        if(bytesReceived < 0)
        {
            printf("\n Read Error in fileno  and value \n");
        }
    
    
    /*n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    }*/

    return 0;
}
