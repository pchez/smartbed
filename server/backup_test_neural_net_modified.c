#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "floatfann.h"
#include "server.h"
#include "9DOF.h"
#include "LSM9DS0.h"

#define PORTNO 5000

#define NUMDATAPTS 151
#define PI 3.14159265358979
static volatile int run_flag = 1;
static volatile int collect_flag;
int shared_array_index = 0;
pthread_mutex_t lock;

struct Angle_Buffer{
    float pitchBuffer[150];
    float rollBuffer[150];
};

struct Avg_Buffer
{	float pitch;
	float roll;
	
};
typedef struct {
    CONNECTION *client;
    int index;
} clients;
int clientCount = 0;
//clientThreads->client[i]


struct Angle_Buffer Angle_Buffer_server;
struct Avg_Buffer Avg_Buffer_client[3];
struct Avg_Buffer Avg_Buffer_server;

void do_when_interrupted()
{
	run_flag = 0;
	pthread_mutex_destroy(&lock);
	printf("\n Threads exiting. Please wait for cleanup operations to complete...\n");
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

typedef struct client_data 
{
	char* clientIP;
	float accel_data_x[5];
	float accel_data_y[5];
	float accel_data_z[5];
	
	float gyro_data_x[5];
	float gyro_data_y[5];
	float gyro_data_z[5];
	
	
} client_data;

client_data sensors = {.clientIP = "ip"};

void* manage_9dof(void *arg) 
{
	NINEDOF *ninedof;
	double sec_since_epoch;

	mraa_init();
	ninedof = ninedof_init(A_SCALE_4G, G_SCALE_245DPS, M_SCALE_2GS);
	float angle_1;
	float angle_2;
	 int i=0;
	 float sum_roll=0;
	 float sum_pitch=0;
	 	for(i=0;i<150;i++)
	 	{
		// timestamp right before reading 9DOF data
		sec_since_epoch = timestamp();
		ninedof_read(ninedof);
		
		//store 9dof data into struct
		sensors.accel_data_x[4] = ninedof->accel_data.x;
		sensors.accel_data_y[4] = ninedof->accel_data.y;
		sensors.accel_data_z[4] = ninedof->accel_data.z;

		sensors.gyro_data_x[4] = ninedof->accel_data.x;
		sensors.gyro_data_y[4] = ninedof->accel_data.y;
		sensors.gyro_data_z[4] = ninedof->accel_data.z;
		angle_1= -2*atan2(ninedof->accel_data.x,sqrt(pow(ninedof->accel_data.x,2)+pow(ninedof->accel_data.y,2)+pow(ninedof->accel_data.z,2)))*(180/PI);
		angle_2=2*atan2(ninedof->accel_data.y,sqrt(pow(ninedof->accel_data.x,2)+pow(ninedof->accel_data.y,2)+pow(ninedof->accel_data.z,2)))*(180/PI);
		//you can put mag as the parameter too.
		//printf("here: index: %d\n", i);
		sum_pitch=sum_pitch+angle_1;
		sum_roll=sum_roll+angle_2;
		//printf("here1\n");
		//Angle_Buffer_server.rollBuffer[i]=angle_2;
		
		usleep(10000);
	}
	Avg_Buffer_server.pitch=sum_pitch/150;
	Avg_Buffer_server.roll=sum_roll/150;

	return NULL;
}

void* handle_client(void *arg) //, float pitchBuffer[], float rollBuffer[])
{
       
	clients *client_struct;
	//clients *clientVal = calloc(1,sizeof(CONNECTION*));
    	int n, client_socket_fd;
	char cmd[256]; //char buffer[256]
	float *buffer;
	buffer = calloc(2,sizeof(float));
	char ready_buf[10];
	collect_flag = 1;	
	//client = (CONNECTION *)arg;
	client_struct = (clients*)arg;
	client_socket_fd = client_struct->client->sockfd;
	//client_socket_fd = clientVal->client[clientCount]->sockfd;
	ioctl(client_socket_fd, FIONBIO, 0);  
	
	//memset(buffer, 0, 256);
	
	int index = client_struct->index;			//private variable - each thread will have its own copy. this is the array index of client_data's arrays in which this thread will store into 
	
	
	 while(collect_flag) {
		int i;
	    	//memset(buffer, 0, 256);
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "pitch");							
		//n = read(client_socket_fd, buffer, 10);		//read from socket to see if client ready
		n = read(client_socket_fd, ready_buf, 10);
		if (n < 0) {
			server_error("ERROR reading from socket");
			printf("client not ready\n");
		}
		else {
			//printf("client sent us: %s\n", buffer);
			if (strcmp(ready_buf, "ready")==0) 
			{	
			        collect_flag = 0;
			        //if we read that the client is ready
				cmd[strlen(cmd)] = '\0';							
				n = write(client_socket_fd, cmd, strlen(cmd));//write 'pitch' to socket to request pitch data
				if (n < 0) {
				    server_error("ERROR writing to socket");
				}
				printf("client is ready, sent pitch command\n");

				printf("Reading pitch data from client\n");
				n = read(client_socket_fd, buffer, 8);
				
				printf("Post read from client\n");
				Avg_Buffer_client[index].pitch=buffer[0];
				Avg_Buffer_client[index].roll=buffer[1];

				if(n < 0) {
				    //printf("HERE!\n");
				    server_error("ERROR reading from socket");
				    //return NULL;
				}

				// end of case statement
			}

				
			
			}
		
		
		if (n == 0) { 
			printf("%s has terminated the connection.\n", client_struct->client->ip_addr_str);
			//printf("%s has terminated the connection.\n",clientVal->client[clientCount]->ip_addr_str);
		    	return NULL;
		}
		
	}

	
	//pthread_exit((void*)client);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t manage_9dof_tid; //, manage_server_tid;
	int rc;
	fann_type *output;
	fann_type input[4];
	struct fann *ann;
	int k, max, patient_location;

	signal(SIGINT, do_when_interrupted);

	//initialize the mutex
	if (pthread_mutex_init(&lock, NULL)!=0)
	{
		printf("mutex init failed\n");
		return 1;
	}

	CONNECTION *server;
	pthread_t tids[256];

	//server thread
	server = (CONNECTION *) server_init(PORTNO, 10);
	if ((int) server == -1){
		run_flag = 0;
	}

	//clients *clientThreads = calloc(4,sizeof(CONNECTION*));	
	
	clients *clientThread_0 = calloc(1,sizeof(clients));
	clients *clientThread_1 = calloc(1,sizeof(clients));
	clients *clientThread_2 = calloc(1,sizeof(clients));
	//clients *clientThread_3 = calloc(1,sizeof(clients));

	clientThread_0->index = 0;
	clientThread_1->index = 1;
	clientThread_2->index = 2;
	//clientThread_3->index = 3;

	clientThread_0->client = (CONNECTION*) server_accept_connection(server->sockfd);
	clientThread_1->client = (CONNECTION*) server_accept_connection(server->sockfd);
	clientThread_2->client = (CONNECTION*) server_accept_connection(server->sockfd);
	//clientThread_3->client = (CONNECTION*) server_accept_connection(server->sockfd);

	//clientThreads->client[0] = (CONNECTION*) server_accept_connection(server->sockfd);
	//clientThreads->client[1] = (CONNECTION*) server_accept_connection(server->sockfd);
	//clientThreads->client[2] = (CONNECTION*) server_accept_connection(server->sockfd);
	//clientThreads->client[3] = (CONNECTION*) server_accept_connection(server->sockfd);

            float server_pitch_avg = 0;
	    float server_roll_avg = 0;
	    float server_pitch_sum = 0; 
	    float server_roll_sum = 0;
	    float client0_pitch_avg = 0;
	    float client0_roll_avg = 0;
	    float client0_pitch_sum = 0;
	    float client0_roll_sum = 0;
	    float client1_pitch_avg = 0;
	    float client1_roll_avg = 0;
	    float client1_pitch_sum = 0;
	    float client1_roll_sum = 0;
	    float client2_pitch_avg = 0;
	    float client2_roll_avg = 0; 
	    float client2_pitch_sum = 0;
	    float client2_roll_sum = 0;
	


		int i;
	while(run_flag) {
	    pthread_create(&tids[0], NULL, handle_client, (void*)clientThread_0);
	    pthread_create(&tids[1], NULL, handle_client, (void*)clientThread_1);
	    pthread_create(&tids[2], NULL, handle_client, (void*)clientThread_2);
	    //pthread_create(&tids[3], NULL, handle_client, (void*)clientThread_3);
	    /*
	    pthread_create(&tids[0], NULL, handle_client, (void*)clientThreads->client[0]);
	    pthread_create(&tids[1], NULL, handle_client, (void*)clientThreads->client[1]);
	    pthread_create(&tids[2], NULL, handle_client, (void*)clientThreads->client[2]);
	    pthread_create(&tids[3], NULL, handle_client, (void*)clientThreads->client[3]);
	    */
	    //
	    clientCount = 0; //reset client count    
	    rc = pthread_create(&manage_9dof_tid, NULL, manage_9dof, NULL);
	    if (rc != 0) {
		fprintf(stderr, "Failed to create manage_9dof thread. Exiting Program.\n");
		exit(0);
	    }
	    
	    pthread_join(manage_9dof_tid, NULL);
	    pthread_join(tids[0], NULL);
	    pthread_join(tids[1], NULL);
	    pthread_join(tids[2], NULL);
	    //pthread_join(tids[3], NULL);
   	
	 
	     server_pitch_avg = 0;
	     server_roll_avg = 0;
	     server_pitch_sum = 0; 
	     server_roll_sum = 0;
	     client0_pitch_avg = 0;
	     client0_roll_avg = 0;
	     client0_pitch_sum = 0;
	     client0_roll_sum = 0;
	     client1_pitch_avg = 0;
	     client1_roll_avg = 0;
	     client1_pitch_sum = 0;
	     client1_roll_sum = 0;
	     client2_pitch_avg = 0;
	     client2_roll_avg = 0; 
	     client2_pitch_sum = 0;
	     client2_roll_sum = 0;
	    //float client3_pitch_avg, client3_roll_avg, client3_pitch_sum, client3_roll_sum;
	    //float client4_pitch_avg, client4_roll_avg, client4_pitch_sum, client4_roll_sum;

	    //float clients_pitch_avg[4], clients_roll_avg[4];	

	    
	   
	   /* for(i=0; i < 150; i++) {
		    server_pitch_sum += Angle_Buffer_server.pitchBuffer[i];
		    server_roll_sum += Angle_Buffer_server.rollBuffer[i];
		    client0_pitch_sum += Angle_Buffer_client_0.pitchBuffer[i];
		    client0_roll_sum += Angle_Buffer_client_0.rollBuffer[i];
		    client1_pitch_sum += Angle_Buffer_client_1.pitchBuffer[i];
		    client1_roll_sum += Angle_Buffer_client_1.rollBuffer[i];
		    client2_pitch_sum += Angle_Buffer_client_2.pitchBuffer[i];
		    client2_roll_sum += Angle_Buffer_client_2.rollBuffer[i];


		    //client3_pitch_sum += Angle_Buffer_client_3.pitchBuffer[i];
		    //client3_roll_sum += Angle_Buffer_client_3.rollBuffer[i];


	
	    }*/

		    /*
		    Avg_Buffer_client[3];
struct Avg_Buffer Avg_Buffer_server;
		    */
 
	   		server_pitch_sum = Avg_Buffer_server.pitch;
		    server_roll_sum =  Avg_Buffer_server.roll;
		    client0_pitch_sum = Angle_Buffer_client[0].pitch;
		    client0_roll_sum = Angle_Buffer_client[0].roll;
		    client1_pitch_sum = Angle_Buffer_client[1].pitch;
		    client1_roll_sum = Angle_Buffer_client[1].roll;
		    client2_pitch_sum = Angle_Buffer_client[2].pitch;
		    client2_roll_sum = Angle_Buffer_client[2].roll;

	    		
	    
	    server_pitch_avg = (server_pitch_sum+90)/180;
	    server_roll_avg = (server_roll_sum+90)/180;
	    client0_pitch_avg = (client0_pitch_sum+90)/180;
	    client0_roll_avg = (client0_roll_sum+90)/180;
	    client1_pitch_avg = (client1_pitch_sum+90)/180;
	    client1_roll_avg = (client1_roll_sum+90)/180;
	    client2_pitch_avg = (client2_pitch_sum+90)/180;
	    client2_roll_avg = (client2_roll_sum+90)/180;
	    //client3_pitch_avg = (client3_pitch_sum/150+90)/180;
	    //client3_roll_avg = (client3_roll_sum/150+90)/180;
	    
	    //test the neural network
	    ann = fann_create_from_file("TEST.net");
	    
	    input[0] = server_pitch_avg;
	    input[1] = server_roll_avg;
	    input[2] = client0_pitch_avg;
	    input[3] = client0_roll_avg;
	    input[4] = client1_pitch_avg;
	    input[5] = client1_roll_avg;
	    input[6] = client2_pitch_avg;
	    input[7] = client2_roll_avg;
	    //input[8] = client3_pitch_avg;
	    //input[9] = client3_roll_avg;
	    

	    output = fann_run(ann, input);
	    max = output[0];
	    patient_location = 0;
	    for (k=1; k<7; k++) {
		    if (output[k] > max) {
			    max = output[k];
			    patient_location = k;
		    }
	    }
	    printf("Patient is at location %d\n", patient_location); 
	
	}

	close(clientThread_0->client->sockfd);
	close(clientThread_0->client->sockfd);
	close(clientThread_0->client->sockfd);

	printf("\n...cleanup operations complete. Exiting main.\n");

	return 0;
} 
