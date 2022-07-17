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
	// write opcode to server
	if ((writen(socket_desc, &buffer[0], 1)) < 0)
	{
		printf("\tFailed to write opcode to server.\n");
		return;
	}
	len = strlen(cmd_path);
	convertedlen = htons(len);
	memcpy(&buffer[1], &convertedlen, 2);
	// write file length to server
	if ((writen(socket_desc, &buffer[1], 2)) < 0)
	{
		printf("\tFailed to write file length to server.\n");
		return;
	}
	// write file path to server
	if ((writen(socket_desc, &cmd_path[0], len)) < 0)
	{
		printf("\tFailed to write file path to server.\n");
		return;
	}
	memset(buffer, 0, MAX_BLOCK_SIZE);
	// read opcode
	if ((readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE)) < 0)
	{
		printf("\tFailed to read opcode from server.\n");
		return;
	}
	memcpy(&opcode, &buffer[0], 1);
	if (opcode != OP_CD)
	{
		printf("\tInvalid op code from server.\n");
		return;
	}
	if ((readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE)) < 0)
	{
		printf("\tFailed to read status from server.\n");
		return;
	}
	memcpy(&status, &buffer[1], 1);
	if (status == SUCCESS_CODE)
	{
		printf("\tCD to new directory: %s.\n", cmd_path);
		return;
	}
	else if (status == ERROR_CODE)
	{
		printf("\tFailed to CD to new directory.\n");
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
}

// download file from server to client
void cli_get(int socket_desc, char *file_name)
{
	return;
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
