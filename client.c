//cash register client
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<signal.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<ctype.h>

#define SERVER_PORT 27032
#define MAXLINE 100

int command_process(int); //client places request with this command
void command_format(void);
void signal_handler(int sig); //programmer-defined signal handler for ctrl+C

int sockfd; //client socket descriptor

int main(int argc, char *argv[])
{
	int check;
	struct sockaddr_in ServAddr;

	//creating the socket
	sockfd=socket(AF_INET,SOCK_STREAM,0); //socket(internet_family,socket_type,protocol_value) retruns socket descriptor
	if(sockfd<0)	
	{
		perror("Cannot create socket!");
		return 0;
	}

	//bzero(&ServAddr,sizeof(ServAddr)); //writes n no. of null nbytes to specified location
	
	//initializing the server socket
	ServAddr.sin_family=AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //using the local system IP (look back address)
	ServAddr.sin_port = htons(SERVER_PORT); //self defined server port

	if((connect(sockfd,(struct sockaddr *) &ServAddr,sizeof(ServAddr)))<0)
	{
		perror("Server is down!");
		return 0;
	}

	signal(SIGINT,signal_handler);

	printf("\n connection is established :\n ");

	while(1)
	{
		
		check=command_process(sockfd);
		if(check<0)
		{
			printf("client closing socket!..exiting!\n");
			close(sockfd);
			exit(0);
		}
		else if(check==1)
		{
			close(sockfd);
			exit(0);
		}
	}

	return 0;
}

void command_format(void)
{
	printf("\nEnter command in the following format:\n<request type> <item code> <quantity>\n");
}


int command_process(int sockfd)
{
	char com_buff[MAXLINE],recv_buff[MAXLINE];
	command_format();

	while(1)
	{
		memset(com_buff,0,MAXLINE);
		memset(recv_buff,0,MAXLINE);

		fgets(com_buff,MAXLINE,stdin);
		
		send(sockfd,com_buff,MAXLINE,0);

		recv(sockfd,recv_buff,MAXLINE,0);
	
		if(recv_buff[0]=='0')
			printf("\n%s\n",recv_buff);

		if(recv_buff[0]=='1' || recv_buff[0]=='4')
		{
			printf("\n");
			fputs(recv_buff,stdout);
			close(sockfd);
			return 1;
		}			

		if(recv_buff[0]=='2')
		{
			printf("\n");
			fputs(recv_buff,stdout);
			command_format();			
			continue;
		}
		
		if(recv_buff[0]=='3')
		{
			printf("Fatal Error!exiting..\n");
			close(sockfd);
			return -1;
		}	
	}	
}

void signal_handler(int sig)
{
	char com_buff[MAXLINE];

	fputs("\nChild Terminating!!..",stdout);

	sprintf(com_buff,"-256");	
	send(sockfd,com_buff,MAXLINE,0);

	close(sockfd);

	exit(0);
}
