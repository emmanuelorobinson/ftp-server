
#include <stdlib.h>         	/* exit(), free() */
#include <stdio.h>          	/* printf(), fprintf(), perror() */
#include <sys/types.h>      	/* pid_t, u_long, u_short */
#include <sys/socket.h>    	/* struct sockaddr, socket(), etc */
#include <netinet/in.h>		/* struct sockaddr_in, htons, htonl */
#include <netdb.h>		/* struct hostent, gethostbyname() */
#include <string.h>         	/* strlen(), strcmp() etc */
#include <stdio.h>		/* stdin(), stdout() */
#include <stdlib.h>		/* exit() */
#include <unistd.h>         	/* read(), write() */

#include "command.h"		/* head file all command function */

#define SERV_TCP_PORT 12345	/* default server listening port */

int main(int argc, char *argv[])
{
	int sd;

    	char host[60];
    	unsigned short port;
    	struct sockaddr_in ser_addr;
    	struct hostent *hp;
    
    	// Source: Chapter 8 Example 6 cli6.c
    	// get server host name and port number
    	if (argc == 1)
    	{  
         	// assume server running on the local host and on default port
         	gethostname(host, sizeof(host));
         	port = SERV_TCP_PORT;
    	}
    	else if (argc == 2)
    	{ 
        	// use the given host name
        	strcpy(host, argv[1]);
        	port = SERV_TCP_PORT;
    	}
    	else if (argc == 3)
    	{ 
        	// use given host and port for server
        	strcpy(host, argv[1]);
        	int n = atoi(argv[2]);
		
        	if (n >= 1024 && n < 65536)
        	{
           		port = n;
        	}
        	else
        	{
            		fprintf(stderr, "Error: server port number must be between 1024 and 65535\n");
            		exit(1);
        	}
    	}
    	else
    	{
        	fprintf(stdout, "Usage: %s [ <server host name> [ <server listening port> ] ]\n", argv[0]);
       		exit(1);
    	}
	
	// Source: Chapter 8 Example 6 cli6.c
    	// get host address, & build a server socket address
    	bzero((char *) &ser_addr, sizeof(ser_addr));
    	ser_addr.sin_family = AF_INET;
    	ser_addr.sin_port = htons(port);
	
    	if ((hp = gethostbyname(host)) == NULL)
    	{
          	fprintf(stdout, "host %s not found\n", host);
          	exit(1);
    	}
	
    	// set the server address
    	ser_addr.sin_addr.s_addr = * (u_long *) hp->h_addr;

    	// Source: Chapter 8 Example 6 cli6.c
    	// create TCP socket & connect socket to server address
    	sd = socket(PF_INET, SOCK_STREAM, 0);
    	if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr)) < 0)
    	{
         	perror("Client connect issue");
         	exit(1);
    	}
    	cmd_prompt(sd);
    	return 0;
}
