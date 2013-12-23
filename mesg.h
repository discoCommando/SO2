#define	MAXMESGDATA 4000

typedef struct {
  long	mesg_type;	
  char	mesg_data[MAXMESGDATA];
} Mesg;

#define	MKEY	108L
#define CLIENTKEY 666L
#define CLIENDKEY 1234L