#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "mesg.h"
#include "err.h"
int main(int argc, char **argv)
{
	if (argc != 4)
		syserr("Wrong number of parameters");
	
	Mesg mesg;
	int id, serv_id, end_id, n_id;
	int n, k, s;
	if ((id = msgget(MKEY, 0)) == -1)
		syserr("msgget");
	if ((serv_id = msgget(CLIENTKEY, 0)) == -1)
		syserr("msgget");
	if ((end_id = msgget(ENDKEY, 0)) == -1)
		syserr("msgget");
	if ((n_id = msgget(NKEY, 0)) == -1)
		syserr("msgget");
// 	WAZNE
	
	k = atoi(argv[1]);
	n = atoi(argv[2]);
	s = atoi(argv[3]);
	
	printf("k %i n %i s %i pid %i\n", k, n, s, getpid());
	
	sprintf(mesg.mesg_data, "%i", k); 
	mesg.mesg_type = (long)(getpid());			/* send messages of this type */
	int temp;
	
	if ((temp = msgsnd(id, (char *) &mesg, n, 0)) != 0)
		syserr("msgsnd");
	
	Mesg second;
	mesg.mesg_type = (long)(getpid());
	sprintf(second.mesg_data, "%i", n); 
	
	if ((temp = msgsnd(n_id, (char *) &mesg, n, 0)	) != 0)
		syserr("msgsnd");
	
	if ((temp = msgrcv(serv_id, &mesg, MAXMESGDATA, (long)(getpid()), 0)) <= 0)
		syserr("msgrcv");
	
	int pid2;
	sscanf(mesg.mesg_data, "%i", &pid2);
	
	fprintf(stdout, "%i %i %i %i", k, n, getpid(), pid2);
	
	sleep(s);
	
	Mesg end;
	sprintf(second.mesg_data, "end\n");
	end.mesg_type = (long)(getpid());
	
	
	if ((temp = msgsnd(end_id, (char *) &mesg, n, 0)) != 0)
		syserr("msgsnd");
	
	return 0;
}
