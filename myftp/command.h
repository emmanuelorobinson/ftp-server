#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>  
#include <fcntl.h>   
#include <dirent.h>	  
#include <netinet/in.h>      

#include "../stream.h"     /* head file for stream read and write */
#include "../netprotocol.h"  /* head file for netprotocol */
#include "../token.h"


#define MAX_NUM_COMMANDS    1000
#define MAX_NUM_CHAR        1000
#define MAX_TOKEN           256
#define BUF_SIZE			256

/** Purpose:	Obtain command from user and process appropriately
 *  Param:		socket descriptor of connection
 *  Return:		void
 *
*/
void cmd_prompt(int socket_desc);

/** Purpose:	To receive and print out the files present in current working directory of the server
 *  Param:		socket descriptor of connection
 *  Return:		void
 *
*/

void extraToken(char *path, int numTok, char *tokenArray[]);

void cli_dir(int socket_desc);

/** Purpose:	To print out the files present in current working directory of client
 *  Return:		void
 *
*/
void cli_ldir();

/** Purpose:	To send and receive the current working directory of the server
 *  Param:		socket descriptor of connection
 *  Return:		void
 *
*/
void cli_pwd(int socket_desc);

/** Purpose:	Print current working directory of client
 *  Return:		void
 *
*/
void cli_lpwd();

/** Purpose:	To change directory of server
 *  Param:		socket descriptor of connection, filepath
 *  Return:		void
 *
*/
void cli_cd(int socket_desc, char* file_path);

/** Purpose:	To change directory of client
 *  Param:		filepath
 *  Return:		void
 *
*/
void cli_lcd(char * cmd_path);

/** Purpose:	To send and upload files to the server
 *  Param:		socket descriptor of connection, filename
 *  Return:		void
 *
*/
void cli_put(int socket_desc, char *filename);

/** Purpose:	To download named file from server to client
 *  Param:		socket descriptor of connection, filepath
 *  Return:		void
 *
*/
void cli_get(int socket_desc, char *file_name);

/** Purpose:	To display the help menu
 *  Return:		void
 *
*/
void cli_help();
