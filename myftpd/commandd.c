#include "commandd.h" /* head file all command function */

// log interactions with the clients
void log_message(char *file, const char *format, ...)
{
	// variables
	va_list ap;
	pid_t pid = getpid();
	char mytime[100];
	time_t raw_time;
	struct tm *time_info;

	// get the current time
	time(&raw_time);
	time_info = localtime(&raw_time);
	strftime(mytime, sizeof(mytime), "%b %d %R", time_info);

	// create or open the log file
	FILE *fp;
	fp = fopen(file, "a");

	if (fp == NULL)
	{
		perror("Server: Log file");
		exit(0);
	}

	fprintf(fp, "%d %s - ", pid, mytime);

	// reference - https://www.tutorialspoint.com/c_standard_library/c_macro_va_start.htm
	// print all the arguments provided
	va_start(ap, format);
	vfprintf(fp, format, ap);
	va_end(ap);

	fclose(fp);
}

void serve_a_client(int sd, char *file)
{
	int nr;
	char buf[MAX_BLOCK_SIZE];

	while (1)
	{
		/* read data from client */
		if ((nr = readn(sd, buf, sizeof(buf))) <= 0)
			return; /* connection broken down */

		if (buf[0] == OP_PUT)
		{
			ser_put(sd, file);
		}
		else if (buf[0] == OP_GET)
		{
			ser_get(sd, file);
		}
		else if (buf[0] == OP_PWD)
		{
			ser_pwd(sd, file);
		}
		else if (buf[0] == OP_DIR)
		{
			ser_dir(sd, file);
		}
		else if (buf[0] == OP_CD)
		{
			ser_cd(sd, file);
		}
		else
		{
			log_message(file, "Unknown command received\n");
		}
	}
}

// List files in server
void ser_dir(int socket_desc, char *file)
{
	// variables
	char buffer[MAX_BLOCK_SIZE];
	int len, nw, nr;
	char status;
	buffer[0] = OP_DIR;

	DIR *dp;
	struct dirent *direntp;
	int filecount = 0;
	char files[MAX_NUM_TOKENS];
	char tmp[MAX_NUM_TOKENS];

	log_message(file, "[DIR] dir command received.\n");

	if ((dp = opendir(".")) == NULL)
	{
		log_message(file, "[DIR] Failed to open directory.\n");
		status = ERROR_CODE;
		nw = writen(socket_desc, &status, 1);
		return;
	}

	// get filenames
	while ((direntp = readdir(dp)) != NULL)
	{
		strcpy(tmp, direntp->d_name);

		if (tmp[0] != '.')
		{
			strcat(files, direntp->d_name);
			if (filecount != 0)
			{
				strcat(files, "\n\t");
			}
		}
		filecount++;
		if (filecount > MAX_NUM_TOKENS)
		{
			log_message(file, "[DIR] Exceeded program capacity, truncated.\n");
			break;
		}
	}

	nr = strlen(files);
	len = htons(nr);
	bcopy(&len, &buffer[2], 2);

	if (nr == 0)
		status = ERROR_CODE;
	else
		status = SUCCESS_CODE;

	// write OPCODE to client
	nw = writen(socket_desc, &buffer[0], 1);
	if (nw < 0)
	{
		fprintf(stderr, "[DIR] Error writing OPCODE to client\n");
		log_message(file, "[DIR] Error writing OPCODE to client\n");
		return;
	}

	// write status to client
	nw = writen(socket_desc, &status, 1);
	if (nw < 0)
	{
		fprintf(stderr, "[DIR] Error writing status to client\n");
		log_message(file, "[DIR] Error writing status to client\n");
		return;
	}

	// write files to client
	nw = writen(socket_desc, &buffer[2], 4);
	if (nw < 0)
	{
		fprintf(stderr, "[DIR] Error writing files to client\n");
		log_message(file, "[DIR] Error writing files to client\n");
		return;
	}

	// write files to client
	nw = writen(socket_desc, files, nr);
	if (nw < 0)
	{
		fprintf(stderr, "[DIR] Error writing files to client\n");
		log_message(file, "[DIR] Error writing files to client\n");
		return;
	}

	log_message(file, "[DIR] Files sent to client\n");
	return;

	return;
}

// PWD from client to display current working directory of server
void ser_pwd(int socket_desc, char *file)
{
	int nw, nr, len;
	char buffer[MAX_BLOCK_SIZE];
	char path[MAX_BLOCK_SIZE];
	char status;

	getcwd(path, MAX_BLOCK_SIZE);

	buffer[0] = OP_PWD;
	nr = strlen(path);
	len = htonl(nr);

	log_message(file, "[PWD] command received\n");

	// copy len to buffer[1]
	memcpy(buffer + 1, &len, sizeof(len));

	// write opcode to client
	nw = writen(socket_desc, &buffer[0], sizeof(buffer));
	if (nw < 0)
	{
		fprintf(stderr, "[PWD]Failed to write opcode\n");
		log_message(file, "[PWD] Failed to write opcode\n");
		return;
	}

	// write status to client
	status = SUCCESS_CODE;
	nw = writen(socket_desc, &status, sizeof(status));
	if (nw < 0)
	{
		fprintf(stderr, "[PWD] Failed to write status\n");
		log_message(file, "[PWD] Failed to write status\n");
		return;
	}

	// write path to client
	if ((nw = writen(socket_desc, path, nr)) != nr)
	{
		fprintf(stderr, "[PWD] Failed to write path\n");
		log_message(file, "[PWD] Failed to write path\n");
		return;
	}

	log_message(file, "[PWD] command sent\n");
}

// CD from client to change the directory of server
void ser_cd(int socket_desc, char *file)
{

	log_message(file, "[CD] CD command received.");
	char buffer[MAX_BLOCK_SIZE];
	char path[MAX_BLOCK_SIZE];
	char status;
	int len;
	int chdirready;

	// read the file length from client
	if ((readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE)) < 0)
	{
		log_message(file, "[CD] failed to read file length.\n");
		return;
	}
	else
	{
		log_message(file, "[CD] read file length.\n");
	}

	memcpy(&len, &buffer[0], 2);
	len = ntohs(len);

	// read path from client
	if ((readn(socket_desc, &buffer[2], MAX_BLOCK_SIZE)) < 0)
	{
		log_message(file, "[CD] failed to read file path.\n");
		return;
	}
	else
	{
		log_message(file, "[CD] read file path.\n");
	}

	memcpy(path, &buffer[2], len);
	path[len] = '\0';

	chdirready = chdir(path);

	// Check if chdir was successful
	if (chdirready == 0)
	{
		status = SUCCESS_CODE;
		log_message(file, "[CD] status is ready.\n");
	}
	else if (chdirready == -1)
	{
		status = ERROR_CODE;
		log_message(file, "[CD] status is error.\n");
	}

	memset(buffer, 0, MAX_BLOCK_SIZE);
	buffer[0] = OP_CD;

	// Write OPCODE to client
	if ((writen(socket_desc, &buffer[0], 1)) < 0)
	{
		log_message(file, "[CD] failed to write opcode to client.\n");
		return;
	}

	buffer[1] = status;

	// Write status to client
	if ((writen(socket_desc, &buffer[1], 1)) < 0)
	{
		log_message(file, "[CD] failed to write status to client.\n");
		return;
	}

	log_message(file, "[CD] CD function ended.\n");
	return;
}

// PUT from client to upload files to server
void ser_put(int socket_desc, char *file)
{
	// variables used
	char opcode, ackcode;
	int file_len, fsize, nr, nw, fd, total = 0;
	char filename[MAX_BLOCK_SIZE];
	char buffer[MAX_BLOCK_SIZE];

	// read file name length and convert to host byte order
	readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE);
	memcpy(&file_len, &buffer[0], 2);
	file_len = ntohs(file_len);

	log_message(file, "[PUT] file name length received.\n");
	log_message(file, "[PUT] file name length is %d.\n", file_len);

	// read file name
	readn(socket_desc, &buffer[2], MAX_BLOCK_SIZE);
	memcpy(&filename, &buffer[2], file_len);

	filename[file_len] = '\0';

	log_message(file, "[PUT] file name received.\n");

	// check if file exist on server
	if (access(filename, R_OK) == 0)
	{
		ackcode = FILE_EXIST;
		log_message(file, "[PUT] put clash error.\n");
	}
	else
	{
		ackcode = SUCCESS_CODE;
		log_message(file, "[PUT] put ready.\n");
	}
	// write opcode and ack code to client
	memset(buffer, 0, MAX_BLOCK_SIZE);
	buffer[0] = OP_PUT;
	buffer[1] = ackcode;
	writen(socket_desc, &buffer[0], 1);
	writen(socket_desc, &buffer[1], 1);

	// if ackcode is '0'
	if (ackcode == SUCCESS_CODE)
	{
		// read opcode from client
		memset(buffer, 0, MAX_BLOCK_SIZE);
		// check if can read opcode from client
		if (readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE) < 0)
		{
			log_message(file, "[PUT] failed to read OPCODE from client.\n");
			return;
		}
		memcpy(&opcode, &buffer[0], 1);

		// check if can read file size from client
		if (readn(socket_desc, &buffer[1], MAX_BLOCK_SIZE) < 0)
		{
			log_message(file, "[PUT] failed to read file size from client.\n");
			return;
		}

		memcpy(&fsize, &buffer[1], 4);
		fsize = ntohl(fsize);

		log_message(file, "[PUT] file size received.\n");

		// create file
		if ((fd = open(filename, O_WRONLY | O_CREAT, 0666)) != -1)
		{

			ackcode = SUCCESS_CODE;

			char block[MAX_BLOCK_SIZE];

			memset(block, '\0', MAX_BLOCK_SIZE);

			lseek(fd, 0, SEEK_SET);

			if (fsize < MAX_BLOCK_SIZE)
			{

				nr = readn(socket_desc, block, MAX_BLOCK_SIZE);

				if (nr < 0)
				{
					log_message(file, "[PUT] failed to read file.\n");
					ackcode = ERROR_CODE;
				}

				nw = write(fd, block, fsize);

				if (nw < 0)
				{
					log_message(file, "[PUT] failed to write file.\n");
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
						log_message(file, "[PUT] failed to read file.\n");
						ackcode = ERROR_CODE;
					}

					int leftover = fsize - total;
					if (leftover < MAX_BLOCK_SIZE)
					{

						nw = write(fd, block, leftover);
						if (nw < 0)
						{
							log_message(file, "[PUT] failed to write file.\n");
							ackcode = ERROR_CODE;
						}
					}
					else
					{
						nw = write(fd, block, MAX_BLOCK_SIZE);
						if (nw < 0)
						{
							log_message(file, "[PUT] failed to write file.\n");
							ackcode = ERROR_CODE;
						}
					}

					total += nw;
				}
			}
			log_message(file, "[PUT] file received from client.\n");
		}
		else
		{
			ackcode = ERROR_CODE;
			log_message(file, "[PUT] put failed.\n");
		}

		memset(buffer, 0, MAX_BLOCK_SIZE);
		opcode = OP_DATA;
		memcpy(&buffer[0], &opcode, 1);

		// write opcode to client
		if (writen(socket_desc, &buffer[0], 1) < 0)
		{
			log_message(file, "[PUT] failed to send opcode to client.\n");
			return;
		}

		memcpy(&buffer[1], &ackcode, 1);

		// write ack code to client
		if (writen(socket_desc, &buffer[1], 1) < 0)
		{
			log_message(file, "[PUT] failed to send ackcode to client.\n");
			return;
		}

		close(fd);

		log_message(file, "[PUT] put success.\n");
		return;
	}
}

// GET from client to download named file from server
void ser_get(int socket_desc, char *file)
{
	log_message(file, "[GET] GET command received.");

	char opcode;
	int file_len, fsize, nr, total = 0;
	char filename[MAX_BLOCK_SIZE];
	char buffer[MAX_BLOCK_SIZE];

	// read file name length and convert to host byte order
	readn(socket_desc, &buffer[0], MAX_BLOCK_SIZE);
	memcpy(&file_len, &buffer[0], 2);
	file_len = ntohs(file_len);

	log_message(file, "[GET] file name length received.\n");

	// read file name
	readn(socket_desc, &buffer[2], MAX_BLOCK_SIZE);
	memcpy(&filename, &buffer[2], file_len);

	filename[file_len] = '\0';

	log_message(file, "[GET] file name received.\n");

	FILE *fileDup;
	fileDup = fopen(filename, "r");
	memset(buffer, 0, MAX_BLOCK_SIZE);

	// check if file exist on server
	// log file name and size to server log file

	log_message(file, "[GET] file name: %s\n", filename);

	if (fileDup != NULL)
	{
		buffer[0] = OP_GET;
		buffer[1] = SUCCESS_CODE;

		if (writen(socket_desc, &buffer[0], 1) < 0)
		{
			log_message(file, "[GET] failed to send opcode to client.\n");
			return;
		}

		if (writen(socket_desc, &buffer[1], 1) < 0)
		{
			log_message(file, "[GET] failed to send ackcode to client.\n");
			return;
		}

		log_message(file, "[GET] File exist on server.\n");

		struct stat fst;

		// get file stat
		if (stat(filename, &fst) == -1)
		{
			log_message(file, "[GET] failed to get file stat.\n");
			return;
		}

		// get file size and convert it to network btye order
		memset(buffer, 0, MAX_BLOCK_SIZE);
		opcode = OP_DATA;
		memcpy(&buffer[0], &opcode, 1);

		if (writen(socket_desc, &buffer[0], 1) < 0)
		{
			log_message(file, "[GET] failed to write opcode to client.\n");
		}

		fsize = (int)fst.st_size;
		int templen = htonl(fsize);
		memcpy(&buffer[1], &templen, 4);

		if (writen(socket_desc, &buffer[1], 4) < 0)
		{
			log_message(file, "[GET] failed to write file size to client.\n");
			return;
		}

		int fd = fileno(fileDup);
		char block[MAX_BLOCK_SIZE];
		memset(block, '\0', MAX_BLOCK_SIZE);
		lseek(fd, 0, SEEK_SET);

		// read and write first block of data
		if (fsize < MAX_BLOCK_SIZE)
		{
			// read and write first block of data
			nr = read(fd, block, fsize);
			writen(socket_desc, block, MAX_BLOCK_SIZE);
		}
		else
		{
			while (total < fsize)
			{
				memset(block, '\0', MAX_BLOCK_SIZE);
				lseek(fd, total, SEEK_SET);

				int leftover = fsize - total;
				if (leftover > MAX_BLOCK_SIZE)
				{
					nr = read(fd, block, MAX_BLOCK_SIZE);
				}
				else
				{
					nr = read(fd, block, leftover);
				}

				writen(socket_desc, block, MAX_BLOCK_SIZE);
				total += nr;
			}
		}
		log_message(file, "[GET] File is sent to client.\n");
	}
	else
	{
		buffer[0] = OP_GET;
		buffer[1] = FILE_NOT_EXIST;

		if (writen(socket_desc, &buffer[0], 1) < 0)
		{
			log_message(file, "[GET] failed to send opcode to client.\n");
			return;
		}
		if (writen(socket_desc, &buffer[1], 1) < 0)
		{
			log_message(file, "[GET] failed to send ackcode to client.\n");
			return;
		}

		log_message(file, "[GET] File does not exist on server.\n");
		return;
	}
}