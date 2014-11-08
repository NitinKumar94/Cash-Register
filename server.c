//cash register server
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>

#define SERVER_PORT 27032
#define MAXLINE 100

void signal_handler(int sig); //programmer-defined signal handler for Ctrl+C command

typedef struct _data
{
	int item_code;
	char item_name[MAXLINE];
	double price;
}database;

database *datap; 
int records; //keeps track of number of records in the database

void create_database(char *); //creates database
int check_code(int); //searches and returns item_code supplied in the database and returns accordingly
void childprocess(int,int);

int listensd,connsd; //sd newsd are socket descriptors

int main(int argc, char *argv[])
{
	int Clilen; //Clilen is the length of the client socket, used as a value-result argument
	pid_t childpid; //holds process id of child
	struct sockaddr_in ServAddr, CliAddr; //sockaddr structure for sockets; one for server and the other for client

	if(argc<2)
	{
		printf("\nToo few arguments to server!..exiting");
		exit(0);
	}

	//creating the socket
	listensd=socket(AF_INET,SOCK_STREAM,0); //socket(internet_family,socket_type,protocol_value) retruns socket descriptor
	if(listensd<0)	
	{
		perror("Cannot create socket!");
		return 0;
	}

	//initializing the server socket
	ServAddr.sin_family=AF_INET;
	ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //using the local system IP (loop back address)
	ServAddr.sin_port = htons(SERVER_PORT); //self defined server port

	//checking server ip and port number	
	//printf("Server IP address is %s\n", inet_ntoa(ServAddr.sin_addr));
	//printf("Socket ID=%d, Sever Port=%d\n",sd,SERVER_PORT);		

	//binding socket
	if(bind(listensd,(struct sockaddr *) &ServAddr, sizeof(ServAddr))<0)
	{
		perror("Cannot bind port!");
		return 0;
	}

	//defining number of clients that can connect through SERVER_PORT , listen() indicates that server is ready for connections
	listen(listensd,5); 

	//creating database
	create_database(argv[1]);

	signal(SIGINT,signal_handler);

	//server runs an infinite loop

	for(;;)
	{
		Clilen=sizeof(CliAddr);
		if((connsd=accept(listensd,(struct sockaddr *)&CliAddr,&Clilen))<0)
		{
			perror("Cannot establish connection!");
			return 0;
		}

		if((childpid=fork())==0)
		{
			close(listensd); //child process closes its copy of the listening socket since it is going to service clients through connsd
			
			printf("\nRequest Serviced with child process %d\n",getpid());
			childprocess(connsd,getpid()); //child process services request

			close(connsd); //child closes its version of connsd after computation is done (return from childprocess())
			exit(0); //child terminates
		}

		close(connsd); //parent closes the connected socket and begins listening for more connections				
	}
}

void childprocess(int connsd,int id)
{
	int len,token_ctr,request_type,quantity,item_code,index;
	double total=0.0;
	char buffer[MAXLINE],msg[MAXLINE],*token;
	const char delim[2]=" ";

	while(1)
	{
		len=token_ctr=index=quantity=0;

		memset(msg,0,MAXLINE); //clears contents of msg

		len=recv(connsd,buffer,MAXLINE,0);
		if(len<0)
		{	
			sprintf(msg,"3 : Error receiving command..exiting\n");
			send(connsd,msg,MAXLINE,0);
			close(connsd);
			exit(0);
		}

		if(strcmp(buffer,"-256")==0)
		{
			printf("\nChild Process %d terminated abnormally!..\n");
			close(connsd);
			exit(0);
		}

		token=strtok(buffer,delim);
		
		request_type=atoi(token); //tokenising request type from command
		
		while(token!=NULL)
		{
			token=strtok(NULL,delim);
			if(token_ctr==0)
				item_code=atoi(token);
			if(token_ctr==1)
				quantity=atoi(token);

			token_ctr++;
		}

		if(token_ctr<2)
		{
			sprintf(msg,"2 : Protocol error..discarding packet!Resend request!\n");
			send(connsd,msg,MAXLINE,0);
		}
		else
		{			
			if(request_type==0)
			{
				index=check_code(item_code);
				if(index>=0)
				{			
					total=total+(datap[index].price * quantity);

					sprintf(msg,"0 : Price = %f\tItem name: %s\n",datap[index].price,datap[index].item_name);
					send(connsd,msg,MAXLINE,0);
				}
				else
				{
					sprintf(msg,"2 : UPC code %d not found!Resend request!\n",item_code);
					send(connsd,msg,MAXLINE,0);
				}
			}
			else if(request_type==1)
			{
				sprintf(msg,"1 : Total cost = %f",total);
				send(connsd,msg,MAXLINE,0);
				close(connsd);
				exit(0);
			}
			else
			{
				sprintf(msg,"2 : Protocol error..discarding packet!Resend request!\n");
				send(connsd,msg,MAXLINE,0);
			}
		}
	}
}

void create_database(char *file_name)
{
	FILE *fp;
	const char delim[2]=" "; // delim[0] - for delimiter, delim[1]='\0'
	char *token,line_buff[MAXLINE];
	int token_ctr=0,ctr=0;
		
	fp=fopen(file_name,"r");

	if(fp==NULL)
	{
		printf("\nCannot open %s",file_name);
		return 0;
	}

	fgets(line_buff,MAXLINE,fp); //reads the first line that has the number of entries in the text file
	
	records=atoi(line_buff); //gets number of records
	//printf("\nNUmber of records = %d",records);

	datap=(database *)malloc(records*sizeof(database)); //allocates memory for database

	while(fgets(line_buff,MAXLINE,fp)!=NULL)
	{
		token = strtok(line_buff,delim);
		datap[ctr].item_code = atoi(token); //getting the item code
		token_ctr=0;

		while(token!=NULL)
		{
			token=strtok(NULL,delim);
			
			if(token_ctr==0) //getting the name of item
				strcpy(datap[ctr].item_name,token);
			else if(token_ctr==1)
				datap[ctr].price = atof(token); //get item price
			/*else
			{
				printf("\ntokenisation error..exiting\n");
				return 0;
			}*/

			token_ctr++;
		}

		ctr++;
	}
	
	return 0;
}

int check_code(int item_code)
{
	int ctr;
	
	for(ctr=0;ctr<records;ctr++)
	{
		if(item_code==datap[ctr].item_code)
		{
			return(ctr);
		}
	}

	return(-1);
}

void signal_handler(int sig)
{
	char msg[MAXLINE];	
	
	close(listensd);
	fputs("\nServer terminating!..",stdout);

	sprintf(msg,"4 : Server terminated!\n");
	send(connsd,msg,MAXLINE,0);

	exit(0);
}
