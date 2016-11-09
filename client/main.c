#include "client.h"
#include "9DOF.h"
#include <math.h>
#include <signal.h>
#include <mraa.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

#define CSV_LINE_SIZE 18 //4 for now, change to 50 for actual implementation
typedef unsigned long long llu;

static volatile int run_flag = 1;
pthread_mutex_t m ;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
pthread_cond_t w = PTHREAD_COND_INITIALIZER;
int done=0;
int sent=1;

float pitch_buffer[151];
float roll_buffer[151];

void do_when_interrupted()
{
	run_flag = 0;
}

double timestamp()
{
	// calculate and return the timestamp
	struct timeval tv;
	double sec_since_epoch;

	gettimeofday(&tv, NULL);
	sec_since_epoch = (double) tv.tv_sec + (double) tv.tv_usec/1000000.0;

	return sec_since_epoch;
}


void* read_data(void *arg)
{	NINEDOF *ninedof;
	mraa_init();
	ninedof = ninedof_init(A_SCALE_4G, G_SCALE_245DPS, M_SCALE_2GS);
	float *pitch, *roll;
	printf("data\n");
	while(run_flag)
	{
	pitch=calloc(151,sizeof(float));
	roll=calloc(151,sizeof(float));
	pitch[0]=0;
	roll[0]=1;
	ninedof_read(ninedof,(pitch),(roll));
	int i=0;
	pthread_mutex_lock(&m);
	while(sent==0)
		pthread_cond_wait(&w, &m);
	for  (i = 0; i < 151; i++)
	{
		pitch_buffer[i]=pitch[i];
		roll_buffer[i]=roll[i];
	}
	printf("done \n");
	sent=0;
	done =1;
	pthread_cond_signal(&c);
	printf("signlaed \n");
	pthread_mutex_unlock(&m);
	printf("done2\n");
	free(pitch);
	free(roll);
	}
}

void* client_handle_connection(void *arg)
{	printf("in client \n");
	int n;
	int rc;
	//float *pitch, *roll;
	char buffer[256]="strnings";
	double sec_since_epoch;

	//memset(buffer, 0, 256);
	int i;
	printf("msg from server:\n ");

	int client;
	client=*(int *)arg;
	
	while (run_flag) 
	{
		//memset(buffer, 0, 256);
		
		// timestamp right before read 9DOF data
		sec_since_epoch = timestamp();
		//ninedof_read(ninedof,(pitch),(roll));
		int i;

		//for(i=0;i<151;i++)
		printf("waiting\n");

		rc=pthread_mutex_lock(&m);
		if(rc==EBUSY)
		{printf("lock bsy\n");
		continue;	
		}
		printf("inside\n");
		while(done ==0)
			pthread_cond_wait(&c, &m);
		
		
		printf("raed is %d\n", done);
		for(i=0;i<151;i++)
		printf("%f",pitch_buffer[i]);	
		//n=read(client,buffer,256);
		n=read(client,buffer,4);
		n = write(client,pitch_buffer, 604);
	/*	if (n < 0){
			printf("zero");
			return 0;
		}
		*/
		printf("numbe of bytesi s%d\n",n);

		//memset(buffer, 0, 256);
        	//n=read(client,buffer,604);
		//printf("number of bytes is %d\n",n);
		/*if (n < 0) {
			return 0;
			sent=1;
		}*/
		done=0;
		sent=1;	
		pthread_cond_signal(&w);
		pthread_mutex_unlock(&m);
		
		printf("exited from client\n");
		// print the message from server
		
		//printf("%f\n", ninedof->accel_data.z);
		usleep(10000);
	}
	close(client);
	return 1;
}

int main(int argc, char *argv[])
{
	//NINEDOF *ninedof;
	int client_socket_fd;
	
	signal(SIGINT, do_when_interrupted);

	//mraa_init();

	// 9DOF sensor initialization
	//ninedof = ninedof_init(A_SCALE_4G, G_SCALE_245DPS, M_SCALE_2GS);

	// client initialization
	int *client;
	(client_socket_fd) = client_init(argc, argv);
	client=&client_socket_fd;
	if (client_socket_fd < 0) {
		return -1;
	}
	
	pthread_t manage_9dof_tid, manage_client_tid;
	int rc;

	rc = pthread_create(&manage_9dof_tid, NULL, read_data, NULL);
	if (rc != 0) {
		fprintf(stderr, "Failed to create manage_9dof thread. Exiting Program.\n");
		exit(0);
	}

	rc = pthread_create(&manage_client_tid, NULL, client_handle_connection, (void*)client);
	if (rc != 0) {
		fprintf(stderr, "Failed to create thread. Exiting program.\n");
		exit(0);
	}

	pthread_join(manage_9dof_tid, NULL);
	pthread_join(manage_client_tid, NULL);

	printf("\n...cleanup operations complete. Exiting main.\n");

	

	return 0;
}
