#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "mesg.h"
#include "err.h"
 
int msg_qid, client_qid, client_end_qid;
#define MAXK 100
#define MAXN 1000
pthread_cond_t waiting[MAXK];
pthread_cond_t pair[MAXK];
pthread_mutex_t mutex[MAXK];
pthread_attr_t attr;
int free_resources[MAXK];
int pair_need[MAXK];
int K, N;
int global_temp;

void exit_server(int sig) 
{
	if (msgctl(msg_qid, IPC_RMID, 0) == -1)
		syserr("msgctl RMID");
	if (msgctl(client_qid, IPC_RMID, 0) == -1)
		syserr("msgctl RMID");
	if (msgctl(client_end_qid, IPC_RMID, 0) == -1)
		syserr("msgctl RMID");
	exit(0);
}

void make_queues()
{
	
	if ((msg_qid = msgget(MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
		syserr("msgget");
	if ((client_qid = msgget(CLIENTKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
		syserr("msgget");
	
// 	if ((client_end_qid = msgget(CLIENDKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
// 		syserr("msgget");

	
}

int get_pid()
{
	int temp, pid;
	Mesg mesg;
	
	if ((temp = msgrcv(msg_qid, &mesg, MAXMESGDATA, 0L, 0)) < 0)
			syserr("msgrcv");
	mesg.mesg_data[temp] = '\0';		/* null terminate filename */
	sscanf(mesg.mesg_data, "%i", &pid);	
	return pid;
}

void init()
{
	int i = 0;
	int err_id;
	for(i = 0; i <= K; i++)
	{
		free_resources[i] = N;
		pair_need[i] = 0;
		
		if ((err_id = pthread_mutex_init(&(mutex[i]), 0) != 0))
			syserr("%d. Mutex init %d failed", err_id, i);
		if ((err_id = pthread_cond_init(&(waiting[i]), 0)) != 0)
			syserr("&d Cond init %d failed", err_id, i);
		if ((err_id = pthread_cond_init(&(pair[i]), 0)) != 0)
			syserr("&d Cond init %d failed", err_id, i);
	}
	if ((err_id = pthread_attr_init(attr)) != 0 )
		syserr("%d. Attr init failed", err_id);
	
	if ((err_id = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED)) != 0)
		syserr("%d. Set Detach faild", err_id);
}

void destroy()
{
	int i = 0;
	int err_id;
	for(i = 0; i <= K; i++)
	{
		free_resources[i] = N;
		pair_need[i] = 0;
		
		if ((err_id = pthread_mutex_destroy(&(mutex[i])) != 0))
			syserr("%d. Mutex init %d failed", err_id, i);
		if ((err_id = pthread_cond_destroy(&(waiting[i]))) != 0)
			syserr("&d Cond init %d failed", err_id, i);
		if ((err_id = pthread_cond_destroy(&(pair[i]))) != 0)
			syserr("&d Cond init %d failed", err_id, i);
	}
}

void *cThread(void *data)
{
        
        //pthread_t pthread_self();
        int pid = *(int *)data;
        int k, n;
		/*
        long msgType = clientPid;
        getRequest(msgType, &k, &n);
        fprintf(stderr,"Received request\n");
        pid_t otherPid = getPartner(clientPid, k, n);
        if (otherPid == clientPid) {
                sendErrorInfo(msgType);
                fprintf(stderr,"Sent Error message\n");
        } else {
                sendResources(msgType, otherPid);
                getResourcesBack(msgType, k, n);
                fprintf(stderr,"Received resources\n");
        }*/
		
        global_temp = pid;
       // threadEnd();
        return 0;
}

void create_thread(int pid)
{
        int temp;
        pthread_t thread;
        if ((temp = pthread_create(&thread, attr, cThread, (void *)&pid)) != 0)
                syserr("%d. Pthread create", temp);
        
        fprintf(stderr,"Created New Thread\n");
}

int main(int argc, char **argv)
{
	if (argc != 3)
		syserr("Wrong number of parameters");
        
	if (signal(SIGINT,  exit_server) == SIG_ERR){
		syserr("signal");}

	K = atoi(argv[1]);
    N = atoi(argv[2]);
	
	init();
	make_queues();
	int pid;
	for(;;) {
		
		pid = get_pid();
		
	}
}
