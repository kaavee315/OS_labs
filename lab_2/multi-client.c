#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>


void error(char *msg) {
    perror(msg);
    exit(0);
}

int users;
int think_time;
int total_time;
int portno;
struct sockaddr_in serv_addr;
struct hostent *server;
int rand_no;
int* requests;
double* resp_time;

int check;

void *connection_handler(void* threadid) {

    int threadnum = (int)threadid; //thread number
    while(1) {

    	//fileno - 2 fixed, else choose randomly 
        int fileno=2;
        if(rand_no) {
            fileno=rand()%10000;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0); //Socket for each thread

        if (sockfd < 0) 
            error("ERROR opening socket");

        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
            error("ERROR connecting");

        int n;
        int bytesReceived = 0;
        char recvBuff[1024];
        char buffer[1024];
        bzero(buffer,1024);

        sprintf(buffer,"get files/foo%d.txt",fileno);

        struct timeval t1, t2;
        double elapsed_time;
        gettimeofday(&t1, NULL); // Start measuring response time

        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
             error("ERROR writing to socket");
        bzero(buffer,1024);

        //Uncomment the code below to write the files received and can be checked for correctness 

        // FILE *fp;
        // char filename[256];
        // sprintf(filename,"ran%d_%d",threadnum,fileno);
        // fp = fopen(filename, "ab"); 
        // if(NULL == fp)
        // {
        //     printf("Error opening file");
        //     return 1;
        // }
        
        while((bytesReceived = read(sockfd, recvBuff, 1024)) > 0)
        {   
            // fp = fopen(filename, "ab"); 
            // bytesReceived = read(sockfd, recvBuff, 1023);
            // printf("Bytes received %d in ",bytesReceived);  
            // printf("Thread no : %d ",threadnum);
            // printf("file no : %d\n",fileno );    

            // printf("Buffer received : %s\n",recvBuff);
            // fwrite(recvBuff,1,bytesReceived,fp);
            // fclose(fp);

            int j,count=0;
            for(j=0;j<1024;j++)
            {   if(recvBuff[j]=='\0')count++;
            }
            if(count==1024){
                break;
            }
        }

        if(bytesReceived < 0)
        {
            printf("\n Read Error in fileno %d and value %d \n", fileno, errno);
        }

        close(sockfd);
        requests[threadnum]=requests[threadnum]+1; // maintain the number of requests per thread
        gettimeofday(&t2, NULL);
        resp_time[threadnum]=resp_time[threadnum]+(t2.tv_sec - t1.tv_sec);
        if(check) //if total_time is up, then break (this is done only after the current file is downloaded)
            break;
        sleep(think_time); // sleep for think_time if any
    }
}

int main(int argc, char *argv[])
{
    struct timeval t1, t2;
    double elapsed_time;
    gettimeofday(&t1, NULL);
    srand(time(NULL)); 
    check=0;

    if (argc != 7) {
       fprintf(stderr,"usage %s hostname port no_users time think_time random/fixed, currently argument no = %d \n", argv[0],argc);
       exit(0);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    users = atoi(argv[3]);
    total_time = atoi(argv[4]);
    think_time = atoi(argv[5]);
    portno = atoi(argv[2]);	//Get all the arguments correctly

    char* type = argv[6];
    if(type[0]=='r') {
        rand_no=1;
    }
    else {
        rand_no=0;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    // Keep an array to store the number of requests and response time for each thread

    pthread_t* thread = malloc(sizeof(pthread_t)*users);
    requests = malloc(sizeof(int)*users);
    resp_time = malloc(sizeof(double)*users);
    int k=0;
    for(k=0; k<users; k++)
    {   resp_time[k]=0.0;
        requests[k]=0;
    }

    int i=0;
    for(i=0;i<users;i++) {
        if(pthread_create(&thread[i], NULL, connection_handler, (void *) i) < 0) {
            perror("Thread not created");
            return 1;
        }
    }

    sleep(total_time); // Wait for total_time
    check=1;	//Then stop all the process
    // And join all of them
    for(i=0; i<users;i++) {
        pthread_join(thread[i],NULL);
    }
    
    gettimeofday(&t2, NULL);
    int total_requests=0;
    double tot_resp_time=0;
    for(i=0;i<users;i++) {
        total_requests=total_requests+requests[i];
        tot_resp_time=tot_resp_time+resp_time[i];
    }
    elapsed_time = (t2.tv_sec - t1.tv_sec);
    // elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    
    printf("resp_time : %f \n",tot_resp_time);
    printf("Throughput : %freq/s \n",total_requests/elapsed_time);
    printf("total_requests : %d \n",total_requests);
    printf("elapsed_time = %f\n", elapsed_time);
    printf("avg_resp_time : %f \n",tot_resp_time/total_requests);
    return 0;

}
