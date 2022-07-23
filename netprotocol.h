/* OP CODES for the network specification */
#define OP_PUT	'P'
#define OP_GET	'G'
#define OP_PWD	'W'
#define OP_DIR	'F'
#define OP_CD	'C'
#define OP_DATA	'D'

/* ACK codes for each network specification */
#define SUCCESS_CODE	'0'
#define ERROR_CODE	    '1'

/* define the commands from user
server commands */
#define CMD_PWD 	"pwd"
#define CMD_FDR		"dir"
#define CMD_CD  	"cd"
#define CMD_PUT		"put"
#define CMD_GET		"get"
/* local commands */
#define CMD_LCD		"lcd"
#define CMD_LFDR	"ldir"
#define CMD_LPWD	"lpwd"
#define CMD_HELP	"help"

/* ACK codes for PUT and GET */
#define FILE_EXIST 'E'
#define FILE_NOT_EXIST 'N'

/* ACK codes for DIR */
#define EXCEED_LENGTH   'L'