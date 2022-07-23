#include  <unistd.h>
#include  <sys/types.h>
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
#include "helper.h"
#include "string.h"

int freadn(int sd, char *buf, int bufsize)
{
	int nr = 1;
	int n = 0;
	for (n = 0; (n < bufsize) && (nr > 0); n += nr)
    	{
		if ((nr = read(sd, buf+n, bufsize-n)) < 0)
		{
			return (nr);
		}
	}
	return (n);
}

int fwriten(int sd, char *buf, int nbytes)
{
	int nw = 0;
	int n = 0;
	for (n = 0; n < nbytes; n += nw)
	{
		if ((nw = write(sd, buf+n, nbytes-n)) <= 0)
        	{
            		return (nw);
        	}

	}
  return n;
}

// Source: Chapter 8 stream.c
// Writes the four byte int on the socket.
int write_length(int socket_desc, int len)
{
	// convert to host to network long
	int data = htonl(len);

	if (write(socket_desc,&data, 4) != 4)
	{
		return -1;
	}

	return 1;
}

// Source: Chapter 8 stream.c
// Reads a four byte int off the socket.
int read_length(int socket_desc, int *len)
{
	int data;

	if (read(socket_desc, &data, 4) != 4)
	{
		return -1;
	}
	
	// to network byte order
	*len = ntohl(data);

	return 1;
}