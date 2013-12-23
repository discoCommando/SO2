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
 
int msg_qid, client_qid, client_end_qid, n_qid;
#define MAXK 100
#define MAXN 1000
pthread_cond_t waiting[MAXK];
pthread_cond_t pair[MAXK];	
pthread_cond_t wait_for_pair[MAXK];
pthread_mutex_t mutex[MAXK];
pthread_mutex_t global_mutex;
pthread_attr_t attr;
int free_resources[MAXK];
int pair_need[MAXK];
int how_many_waiting[MAXK];
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
	if (msgctl(n_qid, IPC_RMID, 0) == -1)
		syserr("msgctl RMID");
	exit(0);
}

void make_queues()
{
	
	if ((msg_qid = msgget(MKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
		syserr("msgget");
	if ((client_qid = msgget(CLIENTKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
		syserr("msgget");
	
	if ((client_end_qid = msgget(ENDKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
		syserr("msgget");
	if ((n_qid = msgget(NKEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)//tworze kolejke
		syserr("msgget");
// WAZNEEE
	
}

Client_info get_info()
{
	int temp, pid, k;
	Mesg mesg;
	
	if ((temp = msgrcv(msg_qid, &mesg, MAXMESGDATA, 0L, 0)) < 0)
		syserr("msgrcv");
	sscanf(mesg.mesg_data, "%i", &k);
	Client_info a;
	a.pid = (int)mesg.mesg_type;
	a.k = k;
	return a;
}

void init()
{
	int i = 0;
	int err_id;
	global_temp = 0;
	for(i = 0; i <= K; i++)
	{
		free_resources[i] = N;
		pair_need[i] = 0;
		how_many_waiting[i] = 0;
		
		if ((err_id = pthread_mutex_init(&(mutex[i]), 0) != 0))
			syserr("%i. Mutex init %i failed", err_id, i);
		if ((err_id = pthread_cond_init(&(waiting[i]), 0)) != 0)
			syserr("&d Cond init %i failed", err_id, i);
		if ((err_id = pthread_cond_init(&(pair[i]), 0)) != 0)
			syserr("&d Cond init %i failed", err_id, i);
		if ((err_id = pthread_cond_init(&(wait_for_pair[i]), 0)) != 0)
			syserr("&d Cond init %i failed", err_id, i);
	}
	if ((err_id = pthread_attr_init(&attr)) != 0 )
		syserr("%i. Attr init failed", err_id);
	
	if ((err_id = pthread_mutex_init(&(global_mutex), 0) != 0))
		syserr("%i. Mutex init %i failed", err_id, i);
		
	if ((err_id = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
		syserr("%i. Set Detach faild", err_id);
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
			syserr("%i. Mutex init %i failed", err_id, i);
		if ((err_id = pthread_cond_destroy(&(waiting[i]))) != 0)
			syserr("&d Cond init %i failed", err_id, i);
		if ((err_id = pthread_cond_destroy(&(pair[i]))) != 0)
			syserr("&d Cond init %i failed", err_id, i);
	}
}

void wake_up(int k)
{
	int temp, pid1;
	if ((temp = pthread_mutex_unlock(&(global_mutex)) != 0))
		syserr("%i. Lock failed", temp);
	
	if ((temp = pthread_cond_signal(&(waiting[k]))) != 0)
		syserr("%i. Lock faileda", temp);
// 	if ((temp = pthread_cond_wait(&(wait_for_pair[k]), &(mutex[k]))) != 0)
// 			syserr("%i. Lock failedas", temp);
// 		
	if ((temp = pthread_mutex_lock(&(global_mutex)) != 0))
		syserr("%i. Lock failed", temp);
	
	
	
	
		
	fprintf(stderr,"pid1 %i\n", global_temp);
	//return pid1;
}
void *pair_thread(void *data)
{
	int pid1, pid2, temp;
	int k = *(int*)data;
	fprintf(stdout,"pair thread \n");
	if ((temp = pthread_mutex_lock(&(global_mutex)) != 0))
		syserr("%i. Lock failed", temp);
	
	wake_up(k);
	pid1 = global_temp;
	global_temp = 0;
	
	Mesg mesg;
	int n1, n2;
	if ((temp = msgrcv(n_qid, &mesg, MAXMESGDATA, (long)(pid1), 0)) <= 0)
		syserr("msgrcv");
	
	sprintf(mesg.mesg_data, "%i", n1);
	
	wake_up(k);
	
	if ((temp = pthread_cond_signal(&(wait_for_pair[k]))) != 0)
		syserr("%i. Lock faileda", temp);
	pid2 = global_temp;
	global_temp = 0;
	if ((temp = msgrcv(n_qid, &mesg, MAXMESGDATA, (long)(pid2), 0)) <= 0)
		syserr("msgrcv");
	sprintf(mesg.mesg_data, "%i", n2);
	fprintf(stderr, "odebrana wiadomosc: %i\n", n2);
	Mesg to_pid1;
	Mesg to_pid2;
	
	to_pid1.mesg_type = (long)pid1;
	to_pid2.mesg_type = (long)pid2;
	
	
	sprintf(to_pid1.mesg_data, "%i", pid2); 
	sprintf(to_pid2.mesg_data, "%i", pid1);
	
	if (free_resources[k] < n1 + n2)
	{
		pair_need[k] = n1 + n2;
		
		if ((temp = pthread_mutex_unlock(&(global_mutex)) != 0))
			syserr("%i. Lock failed", temp);
		
		if ((temp = pthread_cond_wait(&(pair[k]), &(mutex[k]))) != 0)
			syserr("%i. Lock failed", temp);
		
		
		
	}else 
	{
		if ((temp = pthread_mutex_unlock(&(global_mutex)) != 0))
			syserr("%i. Lock failed", temp);
	}
	
	if ((temp = msgsnd(client_qid, (char *) &to_pid1, strlen(to_pid1.mesg_data), 0)	) != 0)
		syserr("msgsnd");
	
	if ((temp = msgsnd(client_qid, (char *) &to_pid2, strlen(to_pid2.mesg_data), 0)	) != 0)
		syserr("msgsnd");
	
	if ((temp = msgrcv(client_end_qid, &to_pid1, MAXMESGDATA, (long)(pid1), 0)) <= 0)
		syserr("msgrcv");
	
	if ((temp = msgrcv(client_end_qid, &to_pid2, MAXMESGDATA, (long)(pid2), 0)) <= 0)
		syserr("msgrcv");
	
	if ((temp = pthread_mutex_lock(&(global_mutex)) != 0))
		syserr("%i. Lock failed", temp);
	
	free_resources[k] += n1 + n2;
	pair_need[k] = 0;
	
	if(how_many_waiting[k] >= 2)
	{
		
	}
	
	
	return 0;
}

void *client_thread(void *data)
{
        
	int temp;
	char *client= (char *)data;
	int pid, k;
	sscanf(client, "%i %i", &pid, &k);
	fprintf(stderr,"%i %i\n", pid, k);
	
// 	if ((temp = pthread_mutex_lock(&(mutex[k])) != 0))
// 		syserr("%i. Lock failed", temp);
// 	
// 	
// 	if ((temp = pthread_mutex_unlock(&(mutex[k])) != 0))
// 		syserr("%i. Lock failed", temp);
	
	if ((temp = pthread_cond_wait(&(waiting[k]), &(mutex[k]))) != 0)
		syserr("%i. Lock failed", temp);
	
	//pthread_t pthread_self();
	
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
	fprintf(stderr, "jestem w client_thread\n");
	
	
	if ((temp = pthread_mutex_lock(&(global_mutex)) != 0))
		syserr("%i. Lock failedd", temp);
	
	
	
	global_temp = pid;
	fprintf(stderr, "	global_temp = pid = %i = %i;\n", pid, global_temp);
	
	
	if(global_temp != 0){
		if ((temp = pthread_cond_signal(&(wait_for_pair[k]))) != 0)
			syserr("%i. Lock faileddd", temp);
		
	else fprintf(stderr, "czekam na mutexaa");
	}
	if ((temp = pthread_mutex_unlock(&(global_mutex)) != 0))
		syserr("%i. Lock faileddd", temp);
	
	// threadEnd();
	return 0;
}

void create_thread(int pid, int k)
{
        int temp;
        pthread_t thread;
        char data[MAXMESGDATA];
		sprintf(data, "%i %i\n", pid, k);
		
        if ((temp = pthread_create(&thread, &attr, client_thread, (void *)data)) != 0) //UWAGA data lub &data
                syserr("%i. Pthread create", temp);
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
	Client_info client;
	for(;;) {
		
		client = get_info();
		create_thread(client.pid, client.k);
		how_many_waiting[client.k]+= 1;
	
		printf("info: %i %i %i %i\n", client.pid, client.k, how_many_waiting[client.k], pair_need[client.k]);
		int temp;
		if ((temp = pthread_mutex_lock(&(global_mutex)) != 0))
			syserr("%i. Lock failed", temp);
	
		if((how_many_waiting[client.k] >= 2) && (pair_need[client.k] == 0))
		{
			how_many_waiting[client.k] -= 2;
			pthread_t thread;
			fprintf(stderr, "Making new pair\n");
			if ((temp = pthread_create(&thread, &attr, pair_thread, (void *)&client.k)) != 0) //UWAGA data lub &data
                syserr("%i. Pthread create", temp);
		}
		if ((temp = pthread_mutex_unlock(&(global_mutex)) != 0))
			syserr("%i. Lock failed", temp);
	
		
		fprintf(stderr, "End of for loop\n");
	}
}
