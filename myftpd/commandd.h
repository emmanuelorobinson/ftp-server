#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <stdarg.h>	
#include <arpa/inet.h>

#include "../stream.h"     	/* head file for stream read and write */
#include "../netprotocol.h"  /* head file for netprotocol */
#include "../helper.h"      	/* head file for helper function */

#define MAX_TOKEN           	256
#define MAX_NUM_TOKENS 100
#define BUF_SIZE            	256
#define MAX_STRING_LENGTH   	10000
#define LOG_NAME		"/myftpd.log"		/* log file */

/** Purpose:	To log all interactions with the clients
 *  Param:	logfile, string format and arguments
 *  Return:	void
 *
*/void log_message(char *file, const char *format, ...);

/** Purpose:	Process OPCODE received from client
 *  Param:	socket descriptor of connection, IP address, logfile
 *  Return:	void
 *
*/
void serve_a_client(int socket_desc, char *file);

/** Purpose:	To send the list of files in current dir of the server
 *  Param:	socket descriptor of connection, logfile
 *  Return:	void
 *
*/
void ser_fdr(int socket_desc, char *file);

/** Purpose:	To send the current working directory of the server
 *  Param:	socket descriptor of connection, logfile
 *  Return:	void
 *
*/
void ser_pwd(int socket_desc, char *file);

/** Purpose:	To change working directory of the server
 *  Param:	socket descriptor of connection, logfile
 *  Return:	void
 *
*/
void ser_cd(int socket_desc, char *file);

/** Purpose:	To upload files to the server
 *  Param:	socket descriptor of connection, logfile
 *  Return:	void
 *
*/
void ser_put(int socket_desc, char *file);

/** Purpose:	To download file from the server
 *  Param:	socket descriptor of connection, logfile
 *  Return:	void
 *
*/
void ser_get(int socket_desc, char *file);
