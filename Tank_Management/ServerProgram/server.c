/* A simple server in the internet domain using TCP
The port number is passed as an argument 
This version runs forever, forking off a separate 
process for each connection
gcc server2.c -lsocket
*/
#include <my_global.h>
#include <mysql.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

void dostuff(int,MYSQL*); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    MYSQL *con = mysql_init(NULL);
    int sockfd, newsockfd, portno, clilen, pid;
    struct sockaddr_in serv_addr, cli_addr;

    if (con == NULL){
      fprintf(stderr, "mysql_init() failed\n");
      exit(1);
    }

    
  
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
        sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            dostuff(newsockfd,con);
            exit(0);
        }
        else close(newsockfd);
} /* end of while */
return 0; /* we never get here */
    }

/******** DOSTUFF() *********************
There is a separate instance of this function 
for each connection.  It handles all communication
once a connnection has been established.
*****************************************/
    void dostuff (int sock,MYSQL *con)
    {
        int n,i,tID;
        double tLevel;
        char buffer[256],buff[1024],TID[10],TLevel[10],tUnit[10],I[5];
        if (con == NULL){
          fprintf(stderr, "mysql_init() failed\n");
          exit(1);
        }

        if (mysql_real_connect(con, "ameyadb.czcnq9ckjlsh.us-east-1.rds.amazonaws.com", "root", "OjasApt123", "ameyadb", 3306, NULL, 0) == NULL){
            finish_with_error(con);
        }   
        if (mysql_query(con, "DROP TABLE IF EXISTS Tanks")) {
            finish_with_error(con);
        }
  
        if (mysql_query(con, "CREATE TABLE Tanks(SNo INT,TANKID INT, LEVEL DOUBLE, UNIT TEXT)")) {      
            finish_with_error(con);
        }
        while (1) {
            bzero(buffer,256);
            n = read(sock,buffer,255);
            sscanf(buffer,"%s%s%s%s",I,TID,TLevel,tUnit);
            
            i=atoi(I);
            tID=atoi(TID);
            tLevel=atof(TLevel);
            if (n < 0) error("ERROR reading from socket");
            printf("%d %d %.2lf %s\n",i,tID,tLevel,tUnit);
            snprintf(buff, sizeof buff, "INSERT INTO Tanks VALUE('%d','%d','%.2lf','%s');",i,tID,tLevel,tUnit);
            if (mysql_query(con, buff))
            {    
                finish_with_error(con);    
            }
            //printf("%s\n",buffer);
            i++;
        }
    }