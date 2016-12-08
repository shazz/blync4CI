/*

Rest interface for Blync
Code by Shazz
Edit by smarie : support for multiple devices and 'cleaner' process mgt + socket release
Thanks to bhijeet Rastogi (http://www.google.com/profiles/abhijeet.1989) and Ticapix (https://github.com/ticapix/blynux)

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#define CONNMAX 100
#define BYTES 1024
#define DEFAULT_PORT "10000"

/* GLOBALS */

char *ROOT;
int listenfd, clients[CONNMAX], devnum;
void startServer(char *);
void respond(int);
bool debugEnabled;
static bool *timeToStop;

/* blynux interface */
int setColorOnDevice(int device_number, int color_mask);

int main(int argc, char* argv[])
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;    
    
    char port[6];
    strcpy(port,DEFAULT_PORT);

    int slot=0;
    debugEnabled = false;
	// shared memory to tell all processes to stop
    timeToStop = (bool*) mmap(NULL, sizeof *timeToStop, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *timeToStop = false;
    devnum = 0;
    pid_t serverPid;
    

    //Parsing the command line arguments
    while ((c = getopt (argc, argv, "p:d")) != -1)
        switch (c)
        {
            case 'p':
                strcpy(port,optarg);
                break;
            case '?':
                fprintf(stderr,"Wrong arguments given!!!\n");
                exit(1);
            case 'd':
                debugEnabled=true;
                printf("Debug enabled\n");
                break;
            //case 'n':
            //    devnum=atoi(optarg);
            //    break;
            default:
                exit(1);
        }
    
    printf("Server started at port %s%s%s\n","\033[92m", port,"\033[0m");
    
    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i=0; i<CONNMAX; i++) clients[i]=-1;
    startServer(port);
    
    serverPid = getpid();
    if(debugEnabled) fprintf(stdout,"[%d] INFO: I'm the parent process (PID %d)\n", getpid(), serverPid);


    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        if(debugEnabled) fprintf(stdout,"[%d] INFO: I'm listening..\n", getpid());	
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);	// blocking !

        if (clients[slot]<0) fprintf(stderr,"[%d] ERROR: on accept()\n", getpid());
        else
        {
			// only the parent process can spawn children
			if((*timeToStop==false) && (getpid() == serverPid))
			{		
				if(debugEnabled) fprintf(stdout,"[%d] INFO: I'm the parent and I need a child !\n", getpid());			
				if ( fork() == 0 ) // so I'm in the child process (and I identify myself as pid == 0)
				{
					respond(slot);
					if(debugEnabled) fprintf(stdout,"[%d] INFO: I did my job and I can die...\n", getpid());	
					exit(1);
				}
				else
				{
					if(debugEnabled) fprintf(stdout,"[%d] INFO: child spawned....\n", getpid());											
				}							
			}
        }
        

        while ((*timeToStop==false) && (clients[slot]!=-1))
        { 
			if(debugEnabled) fprintf(stdout,"[%d] INFO: Looking for next available slot available %d on %d.\n", getpid(), slot+1, CONNMAX);
			slot = (slot+1)%CONNMAX;
		}  
		if(debugEnabled && (*timeToStop==false)) fprintf(stdout,"[%d] INFO: It's NOT time to stop. Server Pid is %d \n", getpid(), serverPid);
		if(debugEnabled && (*timeToStop==true)) fprintf(stdout,"[%d] INFO: It's time to stop. Server Pid is %d \n", getpid(), serverPid);
        if((*timeToStop==true) && (getpid() == serverPid))
        {
			fprintf(stdout,"[%d] INFO: exiting, current parent process : %d\n", getpid(), serverPid);
			break;
		}	
		
		// do some cleaning
		if(getpid() == serverPid)
		{
			pid_t wpid;
			int status = 0;
			if((wpid = wait(&status), WNOHANG) > 0)
			{
				if(debugEnabled) printf("[%d] INFO: Exit status of %d was %d (%s)\n", getpid(), (int)wpid, status, (status > 0) ? "accept" : "reject");
			} 
			else
			{
				if(debugEnabled) printf("[%d] INFO: no child process has terminated yet\n", getpid());
			} 		
		}
			   
    }

    // close socket
    if(debugEnabled) printf("[%d] INFO: closing socket\n", getpid());
    int msg = shutdown(listenfd,2);
    if(debugEnabled) printf("[%d] INFO: done shutdown, msg %d \n", getpid(), msg);
    msg = close(listenfd);
    if(debugEnabled) printf("[%d] INFO: done close, msg %d \n", getpid(), msg);
    return 0;
}

//Stop server
void stopServer()
{
	if(debugEnabled) fprintf(stdout,"[%d] INFO: Stopping server....\n", getpid());
	*timeToStop = true;
	if(debugEnabled && *timeToStop) fprintf(stdout,"[%d] INFO: It will be time to stop....\n", getpid());
	if(debugEnabled && !*timeToStop) fprintf(stdout,"[%d] INFO: It wont be time to stop....\n", getpid());
}

//start server
void startServer(char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
        fprintf(stderr,"ERROR: on getaddrinfo()\n");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next)
    {
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
		// allows new sockets to reuse address of closed sockets. 
		// Useful to be able to restart process without trouble (e.g. using init.d)
		int reuse = 1;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        fprintf(stderr,"ERROR: socket() or bind() have failed\n");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        fprintf(stderr,"ERROR: on listen()\n");
        exit(1);
    }
}

//client connection
void respond(int n)
{
    char mesg[6000], *reqline[4];
    int rcvd;

    memset( (void*)mesg, (int)'\0', 6000 );
    rcvd=recv(clients[n], mesg, 6000, 0);

    if (rcvd<0)    // receive error
    {
		if(debugEnabled) fprintf(stderr,("ERROR: on recv()\n"));
	}
    else if (rcvd==0) // receive socket closed
    {    
        if(debugEnabled) fprintf(stderr,"WARNING: Client disconnected upexpectedly.\n");

	}
    else    // message received
    {
        if(debugEnabled) printf("Received : %s", mesg);
        reqline[0] = strtok (mesg, " \t\n");
        if ( strncmp(reqline[0], "GET\0", 4)==0 )
        {
            reqline[1] = strtok (NULL, " \t");
            reqline[2] = strtok (NULL, " \t\n");
            if ( strncmp( reqline[2], "HTTP/1.0", 8)!=0 && strncmp( reqline[2], "HTTP/1.1", 8)!=0 )
            {
                write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
            }
            else
            {
				send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
				if(debugEnabled) printf("reqline: %s\n", reqline[1]);
                		//devnum = 0;
				devnum = (int) strtol(reqline[1]+1, &reqline[3],10);
				if(debugEnabled) printf("device number: %i\n", devnum);
				if ( strcmp(reqline[3], "/color/green") == 0 ) 
				{
					if(debugEnabled) printf("set color to green\n");			
					setColorOnDevice(devnum, 0xd);
				}
				else if ( strcmp(reqline[3], "/color/red") == 0 ) 
				{
					if(debugEnabled) printf("set color to red\n");
					setColorOnDevice(devnum, 0xe);
				}
				else if ( strcmp(reqline[3], "/color/blue") == 0 ) 
				{
					if(debugEnabled) printf("set color to blue\n");
					setColorOnDevice(devnum, 0xb);
				}
				else if ( strcmp(reqline[3], "/color/cyan") == 0 ) 
				{
					if(debugEnabled) printf("set color to cyan\n");
					setColorOnDevice(devnum, 0x9);
				}
				else if ( strcmp(reqline[3], "/color/yellow") == 0 ) 
				{
					if(debugEnabled) printf("set color to yellow\n");
					setColorOnDevice(devnum, 0xc);
				}
				else if ( strcmp(reqline[3], "/color/magenta") == 0 ) 
				{
					if(debugEnabled) printf("set color to magenta\n");
					setColorOnDevice(devnum, 0xa);
				}				
				else if ( strcmp(reqline[3], "/color/white") == 0 ) 
				{
					if(debugEnabled) printf("set color to white\n");
					setColorOnDevice(devnum, 0x8);
				}
				else if ( strcmp(reqline[3], "/color/off") == 0 ) 
				{
					if(debugEnabled) printf("clear color\n");
					setColorOnDevice(devnum, 0xf);
				}	
				else if ( strcmp(reqline[1], "/state/shutdown") == 0 ) 
				{
					stopServer();
				}	
				else
				{
					if(debugEnabled) fprintf(stderr,"URI unknown : %s\n", reqline[3]);
					write(clients[n], "HTTP/1.0 404 Not Found\n", 23);
				}			
            }
        }
    }

    //Closing SOCKET
    shutdown (clients[n], SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    close(clients[n]);
    clients[n]=-1;
    
    if(debugEnabled) fprintf(stdout,"[%d] INFO: Reponse sent\n", getpid());	
}
