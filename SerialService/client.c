#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX 128
void error(const char *msg)
{
	perror(msg); // this function returns an error message
	exit(1);     // ends the program, 0 means program ended with sucess

}

int main(int argc , char *argv[])
{
	
	int sockfd ,  portno,n ,end;
	struct sockaddr_in serv_addr ;
	struct hostent *server;

	
	
	char buffer[MAX];
	if (argc<3){                                			// error handling 
		fprintf(stderr,"Port number is not valid\n");
		exit(1);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if (sockfd <0){                              			// error handling 
		error("Error opening socket");
	}
	server=gethostbyname(argv[1]);
	if(server==NULL){
		fprintf(stderr,"No such host\n");

	}
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port=htons(portno);
	
	if(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){

		error("Connection failed");

	}else
		puts("COnnected to server");
	
	while(1){

		if(fgets(buffer,MAX,stdin)<0){
			error("Error reading from stdin");
		}
		n=write(sockfd,buffer,strlen(buffer));
		if(n<0)
			error("ERROR writing to socket");
		bzero(buffer,MAX);
		n=read(sockfd,buffer,MAX);
		if(n<0){
			error("Error to read\n");
		}
		puts("Server:");
		puts(buffer);

		}
	close(sockfd); // close all file descriptors
	return 0;
	
}
