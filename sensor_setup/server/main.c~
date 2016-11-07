#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <signal.h>
#include <pthread.h>

#include "server.h"
#include "9DOF.h"
#include "LSM9DS0.h"

#define PORTNO 5000

static volatile int run_flag = 1;
extern shared_array_index = 0;
pthread_mutex_t lock;

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

	//FILE *fp;
	//fp = fopen("./server_test_data.csv", "w");

	//write header to file "test_data.csv"
	//fprintf(fp, "time (epoch), accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z, temperature");
	//fclose(fp);
	
	// 9DOF sensor initialization
	ninedof = ninedof_init(A_SCALE_4G, G_SCALE_245DPS, M_SCALE_2GS);

	while(run_flag) {
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
		// append 9DOF data with timestamp to file "server_test_data.csv"
		/*fp = fopen("./server_test_data.csv", "a");
		fprintf(fp, "%10.10f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", sec_since_epoch,
		ninedof->accel_data.x, ninedof->accel_data.y, ninedof->accel_data.z,
		ninedof->gyro_data.x - ninedof->gyro_offset.x, 
  		ninedof->gyro_data.y - ninedof->gyro_offset.y, 
  		ninedof->gyro_data.z - ninedof->gyro_offset.z,
  		ninedof->mag_data.x, ninedof->mag_data.y, ninedof->mag_data.z,
  		ninedof->temperature);
		fclose(fp);*/

		// print server 9DOF data
	//	ninedof_print(ninedof);
		usleep(10000);
	}

	return NULL;
}

void* handle_client(void *arg)
{
	CONNECTION *client;
	int n, client_socket_fd;
	char buffer[256], tmp[256];
	
	client = (CONNECTION *)arg;
	client_socket_fd = client->sockfd;
	memset(buffer, 0, 256);
	
	
	int index;			//private variable - each thread will have its own copy. this is the array index of client_data's arrays in which this thread will store into 
	pthread_mutex_lock(&lock);	//begin critical section
	index = shared_array_index;	//assign this shared variable to the private index variable so now each thread keeps track of its own unique index	
	shared_array_index += 1;	//increment the shared variable - first thread to access this variable gets index=0, second thread gets index=1, etc
	pthread_mutex_unlock(&lock);	//end critical section
	
	
	while (run_flag) {
		// clear the buffer
		memset(buffer, 0, 256);

		// read what the client sent to the server and store it in "buffer"
		n = read(client_socket_fd, buffer, 255);	
		// an error has occurred
		if (n < 0) {
			server_error("ERROR reading from socket");
			return NULL;
		}
		// no data was sent, assume the connection was terminated
		/*if (n == 0) { 
			printf("%s has terminated the connection.\n", client->ip_addr_str);
			return NULL;
		}*/
		if (n==0) {
		    printf("Waiting for client data\n");
		}

		// print the message to console
		printf("%s says: %s ||| index: %d\n", client->ip_addr_str, buffer, index);
		
		// send an acknowledgement back to the client saying that we received the message
		
		//no need to send data back to client at this point

		//memset(tmp, 0, sizeof(tmp));
		//sprintf(tmp, "%s sent the server: %s", client->ip_addr_str, buffer);
		//n = write(client_socket_fd, tmp, strlen(tmp));
		/*if (n < 0) {
			server_error("ERROR writing to socket");
			return NULL;
		} */
		

		/*Code below can be used when we know how much data we are sending to the server!*/	
		/*int size_buffer = sizeof(strtok(buffer,","));
		printf("Buffer size in tokens: %d\n", size_buffer);
		//char *p[sizeof(strtok(buffer,","))];
		char *p[size_buffer];
		if(sizeof(p)>0) {
		    p[0] = strtok(buffer,","); //tockenizes by comma
		    printf("First token: %s\n",p[0]);
		    int i;
		    for(i=0; i<size_buffer; i++) {
			p[i] = strtok(NULL,",");
			printf("Token %d: %s\n",i,p[i]);
		    }
		}*/

	  	/*From here to end of current block, we parse buffer received and then convert
		 *the tokens into floats, store the floats in the struct and then perform
		 *some rudimentary addition of the struct values as proof of concept
		 */
		//character array is stored in buffer
		char* p;
		p = strtok(buffer,",");
		if(p) {
    			//printf("\nFirst string: %f\n",atof(p));
			sensors.accel_data_x[index] = atof(p);
			//printf("First string: %f\n", sensors.accel_data_x[index]);
		}
		p = strtok(NULL,",");
		if(p) {
		    	sensors.accel_data_y[index] = atof(p);
    			//printf("Next string: %f\n",sensors.accel_data_y[index]);
		}
				
		//float sum;
	    //printf("SUM: %f\n", sensors.accel_data_x[index]+sensors.accel_data_y[index]);	
	    printf("client_data contains: thread1=[%f, %f] thread2=[%f, %f], thread3=[%f, %f], thread4=[%f, %f], serverData_[%f, %f]\n\n", sensors.accel_data_x[0], sensors.accel_data_y[0], sensors.accel_data_x[1], sensors.accel_data_y[1], sensors.accel_data_x[2], sensors.accel_data_y[2], sensors.accel_data_x[3], sensors.accel_data_y[3], sensors.accel_data_x[4], sensors.accel_data_y[4]);
	}

	close(client_socket_fd);
	return NULL;
}

void* manage_server(void *arg)
{
	CONNECTION *server;
	CONNECTION *client;
	int max_connections;
	int i;
	pthread_t tids[256];

	max_connections = 10;

	server = (CONNECTION *) server_init(PORTNO, 10);
	if ((int) server == -1){
		run_flag = 0;
	}

	i = -1;

	while(i < max_connections && run_flag) {
		client = (CONNECTION*) server_accept_connection(server->sockfd);
		if ((int) client == -1) {
			printf("Latest child process is waiting for an incoming client connection.\n");
		}
		else {
			i++;
			pthread_create(&tids[i], NULL, handle_client, (void *)client);
		}
	}

	if (i >= max_connections) {
		printf("Max number of connections reached. No longer accepting connections. Continuing to service old connections.\n");
	}


	for(; i >= 0; i--) {
		pthread_join(tids[i], NULL);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t manage_9dof_tid, manage_server_tid;
	int rc;

	signal(SIGINT, do_when_interrupted);

	//initialize the mutex
	if (pthread_mutex_init(&lock, NULL)!=0)
	{
		printf("mutex init failed\n");
		return 1;
	}
	
	//create threads
	rc = pthread_create(&manage_9dof_tid, NULL, manage_9dof, NULL);
	if (rc != 0) {
		fprintf(stderr, "Failed to create manage_9dof thread. Exiting Program.\n");
		exit(0);
	}

	rc = pthread_create(&manage_server_tid, NULL, manage_server, NULL);
	if (rc != 0) {
		fprintf(stderr, "Failed to create thread. Exiting program.\n");
		exit(0);
	}

	pthread_join(manage_9dof_tid, NULL);
	pthread_join(manage_server_tid, NULL);

	printf("\n...cleanup operations complete. Exiting main.\n");

	return 0;
}
