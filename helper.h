
int freadn(int fd, char *buf, int bufsize);

int fwriten(int fd, char *buf, int nbytes);

int write_length(int socket_desc, int len);

int read_length(int socket_desc, int *len);
