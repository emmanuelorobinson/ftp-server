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
					cli_dir(socket_desc);
				}
				else if (strcmp(tokenArray[0], CMD_PWD) == 0)
				{
					cli_pwd(socket_desc);
				}
				else if (strcmp(tokenArray[0], CMD_LFDR) == 0)
				{
					cli_ldir();
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
void cli_dir(int socket_desc)
{

	// Variable to store the response from server
	char buffer[MAX_BLOCK_SIZE];
	char ser_files[MAX_BLOCK_SIZE];
	int len;

	memset(buffer, 0, MAX_BLOCK_SIZE);
	memset(ser_files, 0, MAX_BLOCK_SIZE);

	buffer[0] = OP_DIR;

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

	if (!(buffer[0] == OP_DIR))
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
void cli_ldir()
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
	char OPCODE, ackcode;
	int fsize, nr, file_len, total = 0;
	char buffer[MAX_BLOCK_SIZE];

	memset(buffer, 0, MAX_BLOCK_SIZE); // set bufferfer to zero
	char file_name[MAX_BLOCK_SIZE]; // use for storing filename
	strcpy(file_name, filename);		// string copy filename

	FILE *file;										// create file pointer
	file = fopen(file_name, "r"); // open client selected file

	// check if file exist on client
	if (file != NULL)
	{
		// write opcode to server
		buffer[0] = OP_PUT;
		writen(socket_desc, &buffer[0], 1);
		memset(buffer, 0, MAX_BLOCK_SIZE);

		// get file name length
		file_len = strlen(file_name);
		int templen = htons(file_len);
		file_name[file_len] = '\0';
		memcpy(&buffer[0], &templen, 2);

		writen(socket_desc, &buffer[0], 2);
		memcpy(&buffer[2], &file_name, file_len);
		writen(socket_desc, &buffer[2], file_len);

		memset(buffer, 0, MAX_BLOCK_SIZE);

		// read OPCODE from server
		if (readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE) < 0)
		{
			printf("\tClient: Failed to read OPCODE from server.\n");
			return;
		}
		else
		{
			memcpy(&OPCODE, &buffer[0], 1);
		}

		// check if OPCODE is valid
		if (readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE) < 0)
		{
			printf("\tClient: Failed to read ackcode from server.\n");
			return;
		}
		else
		{
			memcpy(&ackcode, &buffer[1], 1);
		}

		// check if ackcode is valid
		if (ackcode == SUCCESS_CODE)
		{
			memset(buffer, 0, MAX_BLOCK_SIZE);
			OPCODE = OP_DATA;
			memcpy(&buffer[0], &OPCODE, 1);

			// write OPCODE to server
			if (writen(socket_desc, &buffer[0], 1) < 0)
			{
				printf("\tClient: Failed to write OPCODE to server\n");
				return;
			}

			// get file size and send to server
			struct stat fst;

			if (stat(file_name, &fst) == -1)
			{
				printf("\tClient: Failed to get file stat\n");
				return;
			}

			// get file size and convert it to network btye order
			fsize = (int)fst.st_size;
			templen = htonl(fsize);
			memcpy(&buffer[1], &templen, 4);

			// check if file size is send to server
			if (writen(socket_desc, &buffer[1], 4) < 0)
			{
				printf("\tClient: Failed to write file size to server\n");
				return;
			}

			// getting file descriptor
			int fd = fileno(file);
			char block[MAX_BLOCK_SIZE];
			memset(block, '\0', MAX_BLOCK_SIZE);
			lseek(fd, 0, SEEK_SET);

			// check if file size is smaller than data bufferfer
			if (fsize < MAX_BLOCK_SIZE)
			{
				// read and write first block of data
				nr = read(fd, block, fsize);
				writen(socket_desc, block, MAX_BLOCK_SIZE);
			}
			else
			{
				// if current sent block of data is smaller than total file size
				while (total < fsize)
				{
					memset(block, '\0', MAX_BLOCK_SIZE);
					lseek(fd, total, SEEK_SET);

					if ((fsize - total) > MAX_BLOCK_SIZE)
					{
						nr = read(fd, block, MAX_BLOCK_SIZE);
					}
					else
					{
						nr = read(fd, block, (fsize - total));
					}

					writen(socket_desc, block, MAX_BLOCK_SIZE);
					total += nr;
				}
			}

			// read server response
			memset(buffer, 0, MAX_BLOCK_SIZE);
			// check if can read opcode from server

			if (readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE) < 0)
			{
				printf("\tClient: Error: Not able to read OPCODE from server 3.\n");
				return;
			}

			memcpy(&OPCODE, &buffer[0], 1);

			// check if can read ack code from server
			if (readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE) < 0)
			{
				printf("\tClient: Error: Not able to read ack code from server 3.\n");
			}
			memcpy(&ackcode, &buffer[1], 1);

			// check if opcode is correct
			if (OPCODE == OP_DATA)
			{

				// check if ackcode is 0, which is file transfer done
				if (ackcode == SUCCESS_CODE)
				{
					printf("\tClient: File is transfer succesfully.\n");
				}
				else if (ackcode == ERROR_CODE)
				{
					printf("\tClient: File failed to transfer succesfully.\n");
				}
			}
		}

		// if ackcode returns to be '1'
		else if (ackcode == FILE_EXIST)
		{
			printf("\tClient: File already exist on server\n");
			return;
		}
	}
	else
	{
		printf("\tClient: File cannot be open.\n");
		return;
	}
	return;
}

// download file from server to client
void cli_get(int socket_desc, char *file_name)
{
	char opcode, ackcode;
	int fsize, nr, nw, file_len, total = 0, fd;
	char buffer[MAX_BLOCK_SIZE];
	memset(buffer, 0, MAX_BLOCK_SIZE);

	char filename[MAX_BLOCK_SIZE]; // use for storing filename
	strcpy(filename, file_name);	 // string copy filename
	buffer[0] = OP_GET;

	if (writen(socket_desc, &buffer[0], 1) < 0)
	{
		printf("\tClient: Failed to write OPCODE to server.\n");
		return;
	}

	memset(buffer, 0, MAX_BLOCK_SIZE);
	file_len = strlen(filename);
	int templen = htons(file_len);

	filename[file_len] = '\0';

	// write file name length to server
	memcpy(&buffer[0], &templen, 2);
	if (writen(socket_desc, &buffer[0], 2) < 0)
	{
		printf("\tClient: Failed to write file length to server.\n");
		return;
	}

	// write file name to server using file length
	memcpy(&buffer[2], &filename, file_len);
	if (writen(socket_desc, &buffer[2], file_len) < 0)
	{
		printf("\tClient: Failed to write file name to server.\n");
		return;
	}

	// read server response
	memset(buffer, 0, MAX_BLOCK_SIZE);
	if (readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE) < 0)
	{
		printf("\tClient: Failed to read OPCODE from server\n");
		return;
	}

	memcpy(&opcode, &buffer[0], 1);
	if (readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE) < 0)
	{
		printf("\tClient: Failed to read ackcode from server\n");
		return;
	}

	memcpy(&ackcode, &buffer[1], 1);
	if (opcode == OP_GET)
	{
		if (ackcode == SUCCESS_CODE)
		{
			printf("\tClient: File is found on server.\n");
			
			memset(buffer, 0, MAX_BLOCK_SIZE);
			if (readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE) < 0)
			{
				printf("\tClient: Failed to read OPCODE from server.\n");
				return;
			}
			memcpy(&opcode, &buffer[0], 1);
			if (opcode == OP_DATA)
			{
				// check if can read file size from client
				if (readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE) < 0)
				{
					printf("\tClient: Failed to read file size from client\n");
					return;
				}

				memcpy(&fsize, &buffer[1], 4);
				fsize = ntohl(fsize);

				// create file
				if ((fd = open(filename, O_WRONLY | O_CREAT, 0666)) != -1)
				{
					ackcode = SUCCESS_CODE;
					char block[MAX_BLOCK_SIZE];
					memset(block, '\0', MAX_BLOCK_SIZE);
					lseek(fd, 0, SEEK_SET);

					if (fsize < MAX_BLOCK_SIZE)
					{
						// read from socket stream to data block
						nr = readn(socket_desc, block, MAX_BLOCK_SIZE);

						// if failed to read set ackcode to '1'
						if (nr < 0)
						{
							printf("Client: Failed to read file\n");
							ackcode = ERROR_CODE;
						}

						nw = write(fd, block, fsize);
						if (nw < 0)
						{
							printf("Client: Failed to write file\n");
							ackcode = ERROR_CODE;
						}
					}
					else
					{

						// if total transfer is less than file size
						while (total < fsize)
						{
							memset(block, '\0', MAX_BLOCK_SIZE);
							lseek(fd, total, SEEK_SET);

							nr = readn(socket_desc, block, MAX_BLOCK_SIZE);
							if (nr < 0)
							{
								printf("Client: Failed to read file\n");
								ackcode = ERROR_CODE;
							}

							int leftover = fsize - total;

							// if leftover is less than max block size
							if (leftover < MAX_BLOCK_SIZE)
							{
								nw = write(fd, block, leftover);
								if (nw < 0)
								{
									printf("Client: Failed to write file\n");
									ackcode = ERROR_CODE;
								}
							}
							else
							{
								// write to fd using the max block size
								nw = write(fd, block, MAX_BLOCK_SIZE);
								if (nw < 0)
								{
									printf("Client: Failed to write file\n");
									ackcode = ERROR_CODE;
								}
							}
							// add to total file count
							total += nw;
						}
					}
				}
				printf("\tClient: File is recieved from server.\n");
			}
			else if (ackcode == FILE_NOT_EXIST)
			{
				printf("\tClient: Error: file is not found on server.\n");
				return;
			}
		}
		else
		{
			printf("\tClient: Server send wrong opcode\n");
			return;
		}
	}
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
