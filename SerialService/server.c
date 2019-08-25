/* Copyright 2019, Issam R.S Almustafa
 * All rights reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Date: 2019-08-9
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "SerialManager.h"
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

/*==================[macros and definitions]=================================*/
#define MAX 255
#define BUFF_RX_SIZE	128
/*==================[internal data declaration]==============================*/
int ret,cmp;
static char ledUpdate[MAX];
static unsigned char TCPBuff[BUFF_RX_SIZE];
static unsigned char bufferSerial[BUFF_RX_SIZE];
static unsigned char Ser2TCP_S[BUFF_RX_SIZE];
pthread_t SerialR;														//Declaring thread pointer for serial read
int sockfd , newsockfd, portno, n, end;									//File descriptor pointers
sigset_t set;
int retx;
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;							//Create a mutex key
int flag=0;
/*==================[internal functions declaration]=========================*/
void *SerialRead(void *arg);											//Read serial thread function declaration
void hanlder (int sig);
/*==================[internal data definition]===============================*/
/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/
void handler(int sig){

	if(sig==SIGINT){													//Check if signal is SIGINT
		puts("SIGINT handled");											//Print message if SIGNINT
		close(sockfd); 													// Close the new file descriptor
		serial_close();													//Close serial
        exit(1);														//End program
		}else{
			puts("SIGTERM handled");           							//Print message if SIGTERM
			close(sockfd); 												// Close the new file descriptor
		    serial_close();												//Close serial
	        exit(1);                                                    //End program

		}


}
/*==================[Start Serial threads here]=================================*/

void *SerialRead(void *arg){										//This thread reads from the serial
		while(1){

		if((retx=serial_receive(bufferSerial,BUFF_RX_SIZE))==0){	//Check if anything was sent from serial										//Check if serial sent anything
			//perror("Could not receive from serial");
			//printf("return TRUE is %d\n",retx);

		}else {
			//printf("return FALSE is %d",retx);
		//	strcpy(Ser2TCP_S,bufferSerial);							//Copy content from serial buffer to another one
		//	strcpy(ledUpdate,">OUT:");								//Making up the format to send to thread 2 to turn on the corresponding led
		//	strcat(ledUpdate,Ser2TCP_S+4);							//Get only certain portion of that buffer then attach it to previous one to form the correct format
		//	puts(ledUpdate);										//Print out the final format to be sent
			//serial_send(ledUpdate,BUFF_RX_SIZE);                   //Send new led state to serial back
		//	bzero(ledUpdate,strlen(ledUpdate));						//Clear buffers
			if(flag==1){
				if(n=write(newsockfd,bufferSerial,strlen(bufferSerial))<0){	//Write to socket if the message sent by the serial port
					perror("Could not write to socket");
				}else{
					puts(bufferSerial);
					bzero(bufferSerial,strlen(bufferSerial));					//Clear buffers
				}
			}else{
				continue;														//Go back again to check if flag is 1
				}
			}
		usleep(10000);															//sleep 10 ms to reduce load on CPU
		}
}


/*==================[external functions definition]==========================*/


/* PRINCIPAL FUNCTION, STARTING POINT WHEN RESET IS CALLED. */
int main(int argc , char *argv[])
{
	serial_open(1,115200);												//Configure serial port USB1 and baudrate

	sigemptyset(&set);													//Start a new empty set
	sigaddset(&set,SIGINT);												//Add SIGINT to this set
	sigaddset(&set,SIGTERM);											//Add SIGTERM to this set
	pthread_sigmask(SIG_BLOCK,&set,NULL);								//Block that

	if(ret =pthread_create(&SerialR,NULL,SerialRead,NULL)<0){			//Create a thread to read from Serial
		perror("Thread creation:");
	}

	if (argc<2){                                						// Verify if arguments number were correct
		fprintf(stderr,"Port number not valid\n");						//Send perror to standard perror file of the process
		exit(1);														//Exit the program
	}
	struct sockaddr_in serv_addr , cli_addr;							// Declaring structure types to store socket information
	socklen_t clilen;													//Declare data type to store address length
	sockfd = socket(AF_INET,SOCK_STREAM,0);								//Create socket and store the pointer in this variable

	if (sockfd <0){                              						// Check if file descriptor socket was created correctly
		perror("perror opening socket");
	}
	bzero((char *)&serv_addr,sizeof(serv_addr));						  //Clear out any address in the address of section of the structure
	portno=atoi(argv[1]);

	serv_addr.sin_family=AF_INET;										   //Assign address family
	serv_addr.sin_addr.s_addr=INADDR_ANY;								   //Assign any internal address
	serv_addr.sin_port=htons(portno);									    //Assign port number

	pthread_sigmask(SIG_UNBLOCK,&set,NULL);							     	//Unblock the set for this current thread or any that is created afterwards
	struct sigaction sa;													//Declare a structure of type signation
	sa.sa_handler = handler;												//Assign the handler
	sa.sa_flags = 0;														//Set flag
	if (sigaction(SIGINT,&sa,NULL)==-1){									//Associate SIGUSR2 to sa structure
	   		perror("Sigaction SIGINT");
	}
	if (sigaction(SIGTERM,&sa,NULL)==-1){									//Associate SIGUSR2 to sa structure
		perror("Sigaction SIGTERM");
	}

	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){		//Bind socket with the information from the structure
		perror("Binding failed");
	}
	listen(sockfd,1); 														//Listen with maximum 1 connection allowed
	clilen=sizeof(cli_addr);												//Assign address size to clilen variable

	while(1){
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,&clilen);		//Accept returns a new socket pointer and it is stored in this variable
		if(newsockfd<0){														//Check if new socket file was created
			perror("Failed to accept");
			continue;															//Go back to if statement if failed to accept and create new socket
		}else{
			puts("Connection accepted\n");
			flag=1;
		}
		while(1){
			n=read(newsockfd,TCPBuff,BUFF_RX_SIZE);
			if(n==0){					   										//Read data from socket and pass it to buffer
				perror("error to read from socket");
				break;
			}else{
				serial_send(TCPBuff,BUFF_RX_SIZE);	                    		 //Write to serial if anything  from socket. this function returns void and is blocking when used with if
				puts(TCPBuff);
				bzero(TCPBuff,strlen(TCPBuff));
			}
			usleep(10000);															//sleep 10 ms to reduce load on CPU
		}
		flag=0;															//Reset flag to accept new connection and avoid infinite loop
		close(newsockfd); 												// Close the new file descriptor
	}
	close(sockfd); 													// Close the new file descriptor
	close(newsockfd); 												// Close the new file descriptor
	serial_close();													//Close serial

	return 0;
	
}
/*==================[end of file]============================================*/
