#define	MAXMESGDATA 4000

typedef struct {
  long	mesg_type;	
  char	mesg_data[MAXMESGDATA];
} Mesg;

typedef struct
{
	int pid;
	int k;
} Client_info;


#define	MKEY	108L

#define CLIENTKEY 666L

#define ENDKEY 1234L

#define NKEY 1235L

#define XD 1232L