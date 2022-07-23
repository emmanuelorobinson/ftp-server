#include "command.h" /* head file all command function */

// send OPCODE to server
void cmd_prompt(int socket_desc)
{
	// declare token on 100 char
	char input[MAX_NUM_CHAR];

	// declare array of size max_token
	char *tokenArray[MAX_TOKEN];

	// declare total num of tokens
	int numTok;

	while (1)
	{
		fprintf(stdout, " > "); // command prompt

		fgets(input, 99, stdin);

		// Remove the return char if present
		rmReturnChar(input);

		if (strcmp(input, "quit") == 0)
		{
			return;
		}
		else
		{
			bzero(tokenArray, sizeof(tokenArray));
			numTok = tokenise(input, tokenArray);

			if (numTok <= 0)
			{
				fprintf(stdout, "Usage: Command [optional: file/dir path]\n");
				continue;
			}

			else if (numTok == 1)
			{
				if (strcmp(tokenArray[0], CMD_FDR) == 0)
				{
					cli_fdr(socket_desc);
				}
				else if (strcmp(tokenArray[0], CMD_PWD) == 0)
				{
					cli_pwd(socket_desc);
				}
				else if (strcmp(tokenArray[0], CMD_LFDR) == 0)
				{
					cli_lfdr();
				}
				else if (strcmp(tokenArray[0], CMD_LPWD) == 0)
				{
					cli_lpwd();
				}
				else if (strcmp(tokenArray[0], CMD_HELP) == 0)
				{
					cli_help();
				}
				else
				{
					fprintf(stdout, "Invalid command, try again. type \"help\" for documentation\n");
				}
			}

			else if (numTok == 2)
			{
				if (strcmp(tokenArray[0], CMD_CD) == 0)
				{
					cli_cd(socket_desc, tokenArray[1]);
				}
				else if (strcmp(tokenArray[0], CMD_PUT) == 0)
				{
					cli_put(socket_desc, tokenArray[1]);
				}
				else if (strcmp(tokenArray[0], CMD_GET) == 0)
				{
					cli_get(socket_desc, tokenArray[1]);
				}
				else if (strcmp(tokenArray[0], CMD_LCD) == 0)
				{
					cli_lcd(tokenArray[1]);
				}
				else
				{
					fprintf(stdout, "Invalid command, try again. type \"help\" for documentation\n");
				}
			}
			else
			{
				fprintf(stdout, "Invalid command, try again. type \"help\" for documentation\n");
			}
		}
	}
}

// list files in server
void cli_fdr(int socket_desc)
{

	// Variable to store the response from server
	char buffer[MAX_BLOCK_SIZE];
	char ser_files[MAX_BLOCK_SIZE];
	int len;

	memset(buffer, 0, MAX_BLOCK_SIZE);
	memset(ser_files, 0, MAX_BLOCK_SIZE);

	
	buffer[0] = OP_FDR;

	// write OPCODE to server
	if ((writen(socket_desc, buffer, 1)) < 0)
	{
		printf("\tClient: Failed to write OPCODE to server.\n");
		return;
	}

	// read OPCODE from server
	if ((readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE)) < 0)
	{
		printf("\tClient: Failed to read OPCODE from server.\n");
		return;
	}

	if (!(buffer[0] == OP_FDR))
	{
		printf("\tClient: Failed to read DIR op code\n");
		return;
	}

	if ((readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE)) < 0)
	{
		printf("\tClient: Failed to read ack code\n");
		return;
	}

	// Check if the server is ready to send the files
	if (buffer[1] == SUCCESS_CODE)
	{

		// read the files from server
		if ((readn(socket_desc, &buffer[2], MAX_BLOCK_SIZE)) < 0)
		{
			printf("\tClient: Failed to read length of server file path\n");
			return;
		}

		// copy 4 bytes of the length
		memcpy(&len, &buffer[2], 2);
		len = (int)ntohs(len);

		// read the server file path
		readn(socket_desc, ser_files, sizeof(buffer));

		// print the server file path
		printf("\t%s\n", ser_files);
	}
	else
	{
		printf("\tClient: Failed: Status code was '1'\n");
	}


	return;
}

// list files in client
void cli_lfdr()
{
	// variables
	char *filearray[MAX_TOKEN];
	int filecount = 0;

	// Open current dir and struct
	DIR *dp;
	struct dirent *direntp;
	if ((dp = opendir(".")) == NULL)
	{
		// set to first entry
		fprintf(stderr, "Client: Failed to open directory\n");
		return;
	}

	// insert the filenames
	while ((direntp = readdir(dp)) != NULL)
	{
		filearray[filecount] = direntp->d_name;
		filecount++;
		if (filecount >= MAX_TOKEN - 1)
		{
			fprintf(stderr, "Client: Exceeded program capacity, truncated\n");
			break;
		}
	}

	for (int i = 0; i < filecount; i++)
	{
		fprintf(stdout, "%s\t", filearray[i]);
	}

	fprintf(stdout, "\n");

	return;
}

// print currect working directory of server
void cli_pwd(int socket_desc)
{
	char buffer[MAX_BLOCK_SIZE];
	char status;
	char path[MAX_BLOCK_SIZE];
	int n;

	memset(buffer, 0, MAX_BLOCK_SIZE);

	buffer[0] = OP_PWD;

	// send OPCODE to server using writen()
	n = writen(socket_desc, buffer, MAX_BLOCK_SIZE);
	if (n < 0)
	{
		fprintf(stderr, "Client: Failed to send OPCODE\n");
		return;
	}

	// read OPCODE from server using readn()
	n = readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE);
	if (n < 0)
	{
		fprintf(stderr, "Client: Failed to read OPCODE\n");
		return;
	}

	n = readn(socket_desc, &status, MAX_BLOCK_SIZE);

	// check if OPCODE is valid
	if (buffer[0] != OP_PWD)
	{
		fprintf(stderr, "Client: Invalid OPCODE\n ");
		return;
	}

	// read the size of the path
	n = readn(socket_desc, path, sizeof(buffer));
	if (n < 0)
	{
		fprintf(stderr, "Client: Failed to read path\n");
		return;
	}

	// print the path
	fprintf(stdout, "%s\n", path);
	return;
}

// print current working directory of client
void cli_lpwd()
{
	char buf[BUF_SIZE];
	getcwd(buf, sizeof(buf));
	fprintf(stdout, "%s\n", buf);
	return;
}

// change directory of server
void cli_cd(int socket_desc, char *cmd_path)
{

	char buffer[MAX_BLOCK_SIZE];
	char opcode, status;
	int len, convertedlen;

	buffer[0] = OP_CD;

	// write OPCODE to server
	if ((writen(socket_desc, &buffer[0], 1)) < 0)
	{
		printf("\tClient: Failed to write opcode to server.\n");
		return;
	}

	len = strlen(cmd_path);
	convertedlen = htons(len);
	memcpy(&buffer[1], &convertedlen, 2);

	// write file length to server
	if ((writen(socket_desc, &buffer[1], 2)) < 0)
	{
		printf("\tClient: Failed to write file length to server.\n");
		return;
	}

	// write file path to server
	if ((writen(socket_desc, &cmd_path[0], len)) < 0)
	{
		printf("\tClient: Failed to write file path to server.\n");
		return;
	}

	memset(buffer, 0, MAX_BLOCK_SIZE);


	// read OPCODE from server
	if ((readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE)) < 0)
	{
		printf("\tClient: Failed to read OPCODE from server.\n");
		return;
	}

	memcpy(&opcode, &buffer[0], 1);

	// check if OPCODE is valid
	if (opcode != OP_CD)
	{
		printf("\tClient: Invalid OPCODE from server.\n");
		return;
	}

	// read status code from server
	if ((readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE)) < 0)
	{
		printf("\tClient: Failed to read status from server.\n");
		return;
	}

	memcpy(&status, &buffer[1], 1);

	// check if status code is valid
	if (status == SUCCESS_CODE)
	{
		printf("\tClient: CD to new directory: %s.\n", cmd_path);
		return;
	}
	else if (status == ERROR_CODE)
	{
		printf("\tClient: Failed to CD to new directory.\n");
		return;
	}
	return;
}

// change directory of client
void cli_lcd(char *cmd_path)
{
	rmReturnChar(cmd_path);

	if (chdir(cmd_path) == -1)
	{
		fprintf(stderr, "Client: Unable to CD to %s\n", cmd_path);
	}
	return;
}

// upload file from client to server
void cli_put(int socket_desc, char *filename)
{
	// variables
	char op_code, ack_code;
	int fd, file_size, nr;
	struct stat stats;
	char buf[BUF_SIZE]; // buffer for file content

	int file_len = strlen(filename); // set file length
	char file_name[file_len + 1]; // define file_name

	strcpy(file_name, filename); // copy filename to file_name

	file_name[file_len] = '\0'; // set last character to null

	// file validation
	if ((fd = open(file_name, O_RDONLY)) == -1)
	{
		fprintf(stderr, "Client: Failed to read source file\n");
		return;
	}

    	// check for fstat
    	if(fstat(fd, &stats) < 0)
    	{
		fprintf(stderr, "Client: Failed to read fstat\n");
        	return;
    	}

	file_size = stats.st_size; // set file size

	// send opcode to server
	if (write_opcode(socket_desc, OP_PUT) == -1)
	{
		fprintf(stderr, "Client: Failed to write opcode\n");
		return;
	}

	// sent file length
	if (write_length(socket_desc, file_len) == -1)
	{
		fprintf(stderr, "Client: Failed to send length\n");
		return;
	}

	// send file name
	if (fwriten(socket_desc, file_name, file_len) < 0)
	{
		fprintf(stderr, "Client: Failed to send filename\n");
		return;
	}

	// read opcode from server
	if (read_opcode(socket_desc, &op_code) == -1)
	{
		fprintf(stderr, "Client: Failed to read opcode\n");
		return;
	}

	// read ackcode from server
	if (read_opcode(socket_desc, &ack_code) == -1)
	{
		fprintf(stderr, "Client: Failed to read ackcode\n");
		return;
	}

	// send file data
	if (ack_code == SUCCESS_CODE)
	{
		if (write_opcode(socket_desc, OP_DATA) == -1)
		{
			fprintf(stderr, "Client: Failed to write opcode\n");
			return;
		}

		if (write_length(socket_desc, file_size) == -1)
		{
			fprintf(stderr, "Client: Failed to send length\n");
			return;
		}

		while ((nr = freadn(fd, buf, BUF_SIZE)) > 0)
		{
			if (writen(socket_desc, buf, nr) == -1)
			{
				fprintf(stdout, "Client: Failed to send file content\n");
				return;
			}
		}
	}
	else if (ack_code == FILE_EXIST)
	{
		fprintf(stdout, "Server: File exist on server. Unable to send\n");
	}
	else if (ack_code == ERROR_CODE)
	{
		fprintf(stdout, "Server: Error in sending file\n");
	}

	close(fd);
}

// download file from server to client
void cli_get(int socket_desc, char *file_name)
{
	// variables
	char op_code, ack_code;
	int fd, file_size, block_size, nr, nw;
	char buf[BUF_SIZE]; // buffer for file content

	int file_len = strlen(file_name);

	// check for file exist or error creating file
	if (access(file_name, F_OK) >= 0)
	{
		fprintf(stderr, "Client: File exists in current folder. Unable to get\n");
		return;
	}

	// send opcode to server
	if (write_opcode(socket_desc, OP_GET) == -1)
	{
		fprintf(stderr, "Client: Failed to write opcode\n");
		return;
	}

	// send file length to server
	if (write_length(socket_desc, file_len) == -1)
	{
		fprintf(stderr, "Client: Failed to write length\n");
		return;
	}

	// send file name to server
	if (fwriten(socket_desc, file_name, file_len) == -1)
	{
		fprintf(stderr, "Client: Failed to write filename\n");
		return;
	}

	// read opcode from server
	if (read_opcode(socket_desc, &op_code) == -1)
	{
		fprintf(stderr, "Client: Failed to read opcode\n");
		return;
	}

	// read ackcode from server
	if (read_opcode(socket_desc, &ack_code) == -1)
	{
		fprintf(stderr, "Client: Failed to read ackcode\n");
		return;
	}

	if ((fd = open(file_name, O_WRONLY|O_CREAT, 0766)) < 0)
	{
		ack_code = ERROR_CODE;
		fprintf(stderr, "Client: Failed to create file\n");
		return;
	}

	if (ack_code == SUCCESS_CODE)
	{
		// read opcode from server
		if (read_opcode(socket_desc, &op_code) == -1)
		{
			fprintf(stderr, "Client: Failed to read ackcode\n");
			return;
		}

		// read the file size
		if (read_length(socket_desc, &file_size) == -1)
		{
			fprintf(stderr, "Client: Failed to read size\n");
			return;
		}

		block_size = BUF_SIZE; // set block size

		while (file_size > 0)
		{
			if (block_size > file_size)
			{
				block_size = file_size;
			}

			if ((nr = freadn(socket_desc, buf, block_size)) == -1)
			{
				fprintf(stdout, "Client: Failed to read data\n");
				return;
			}

			if ((nw = fwriten(fd, buf, nr)) < nr)
			{
				fprintf(stdout, "Client: Failed to write data\n");
				return;
			}
			file_size -= nw;
		}
	}
	else if (ack_code == FILE_NOT_EXIST)
	{
		fprintf(stdout, "Server: File does not exist on server\n");
		unlink(file_name);
	}
	else if (ack_code == ERROR_CODE)
	{
		fprintf(stdout, "Server: Error sending file to client\n");
		unlink(file_name);
	}

	close(fd);
}

// display the help menu information
void cli_help()
{
	printf(
			"Command\t\t\t Function\n"
			"pwd\t\t\t Display current directory of the server\n"
			"lpwd\t\t\t Display current directory of the client\n"
			"dir\t\t\t List files of current directory from the server\n"
			"ldir\t\t\t List files of current directory from the client\n"
			"cd <directory_pathname>  Change directory of the server\n"
			"lcd <directory_pathname> Change directory of the client\n"
			"get <filename>\t\t Download file from the server to client\n"
			"put <filename>\t\t Upload file from client to server\n"
			"quit\t\t\t Terminate session\n"
			"help\t\t\t Display help information\n");
}
