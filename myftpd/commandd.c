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
	int nr, nw;
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
		else if (buf[0] == OP_FDR)
		{
			ser_fdr(sd, file);
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
void ser_fdr(int socket_desc, char *file)
{
	// variables
	char buf[MAX_BLOCK_SIZE];
	int len, nw, nr;
	char status;
	buf[0] = OP_FDR;

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
	bcopy(&len, &buf[2], 2);

	if (nr == 0)
		status = ERROR_CODE;
	else
		status = SUCCESS_CODE;

	// write OPCODE to client
	nw = writen(socket_desc, &buf[0], 1);
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
	nw = writen(socket_desc, &buf[2], 4);
	if (nw < 0)
	{
		fprintf(stderr, "[DIR] Error writing files to client\n");
		log_message(file, "[DIR] Error writing files to client\n");
		return;
	}

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
	char buf[MAX_BLOCK_SIZE];
	char path[MAX_BLOCK_SIZE];
	char status;

	getcwd(path, MAX_BLOCK_SIZE);

	buf[0] = OP_PWD;
	nr = strlen(path);
	len = htonl(nr);

	log_message(file, "[PWD] command received\n");

	// copy len to buf[1]
	memcpy(buf + 1, &len, sizeof(len));

	// write opcode to client
	nw = writen(socket_desc, &buf[0], sizeof(buf));
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
	char buf[MAX_BLOCK_SIZE];
	char path[MAX_BLOCK_SIZE];
	char status;
	int len;
	int chdirready;

	if ((readn(socket_desc, &buf[0], MAX_BLOCK_SIZE)) < 0)
	{
		log_message(file, "[CD] failed to read file length.");
		return;
	}
	else
	{
		log_message(file, "[CD] read file length.");
	}

	memcpy(&len, &buf[0], 2);
	len = ntohs(len);
	if ((readn(socket_desc, &buf[2], MAX_BLOCK_SIZE)) < 0)
	{
		log_message(file, "[CD] failed to read file path.");
		return;
	}
	else
	{
		log_message(file, "[CD] read file path.");
	}

	memcpy(path, &buf[2], len);
	path[len] = '\0';

	chdirready = chdir(path);

	if (chdirready == 0)
	{
		status = SUCCESS_CODE;
		log_message(file, "[CD] status is ready.");
	}
	else if (chdirready == -1)
	{
		status = ERROR_CODE;
		log_message(file, "[CD] status is error.");
	}

	memset(buf, 0, MAX_BLOCK_SIZE);
	buf[0] = OP_CD;

	if ((writen(socket_desc, &buf[0], 1)) < 0)
	{
		log_message(file, "[CD] failed to write opcode to client.");
		return;
	}

	buf[1] = status;

	if ((writen(socket_desc, &buf[1], 1)) < 0)
	{
		log_message(file, "[CD] failed to write status to client.");
		return;
	}

	// printf("[CD] CD function ended.\n");
	log_message(file, "[CD] CD function ended.");
	return;
}

// PUT from client to upload files to server
void ser_put(int socket_desc, char *file)
{
	return;
}

// GET from client to download named file from server
void ser_get(int socket_desc, char *file)
{
	return;
}
