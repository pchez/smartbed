		// print client 9DOF readings 
//#include "client.h"
#include "9DOF.h"
#include <math.h>
#include <signal.h>
#include <mraa.h>
#include <sys/time.h>

#define CSV_LINE_SIZE 18 //4 for now, change to 50 for actual implementation
typedef unsigned long long llu;

static volatile int run_flag = 1;

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

int handle_connection(NINEDOF *ninedof)
{
	int n;
	float *pitch, *roll;
	char buffer[256];
	double sec_since_epoch;

	pitch=calloc(51,sizeof(float));
	roll=calloc(51,sizeof(float));

	memset(buffer, 0, 256);

	printf("msg from server: %s\n", buffer);

	//while (run_flag) 
	{

		memset(buffer, 0, 256);
		
		// timestamp right before read 9DOF data
		sec_since_epoch = timestamp();
		ninedof_read(ninedof,pitch,roll);
		int i;
		for(i=0;i<50;i++)
			printf("%f %f",pitch[i],roll[i]);
		// print client 9DOF readings
 		//ninedof_print(ninedof);
		//calculate client 9DOF pitch and roll
		//pitch = -atan2(ninedof->accel_data.x, sqrt(pow(ninedof->accel_data.x,2)+pow(ninedof->accel_data.y,2)+pow(ninedof->accel_data.z,2)))*180/M_PI;
		//roll = atan2(ninedof->accel_data.y, sqrt(pow(ninedof->accel_data.x,2)+pow(ninedof->accel_data.y,2)+pow(ninedof->accel_data.z,2)))*180/M_PI;
	FILE* fp;	
		if((fp=fopen("client_data", "wb"))==NULL) {
    	printf("Cannot open file.\n");
  		}

  		if(fwrite(pitch, sizeof(float), 51, fp) != 51)
    		printf("File read error.");
  		fclose(fp);
		//read from csv file and then clear it
		//write pitch and roll to buffer
		//sprintf(buffer, "%f, %f", pitch, roll);

		// send 9DOF reading to server
		/*n = write(client_socket_fd, pitch, strlen(buffer));
		if (n < 0) {
			return client_error("ERROR writing to socket");
		}

		memset(buffer, 0, 256);

		// get response message from server
		n = write(client_socket_fd, roll, 255);
		if (n < 0) {
			return client_error("ERROR writing from socket");
		}*/

		// print the message from server
		
		//printf("%f\n", ninedof->accel_data.z);
		usleep(10000);
	}
	//close(client_socket_fd);
	return 1;
}

int main(int argc, char *argv[])
{
	NINEDOF *ninedof;
	int client_socket_fd;

	signal(SIGINT, do_when_interrupted);

	mraa_init();

	// 9DOF sensor initialization
	ninedof = ninedof_init(A_SCALE_4G, G_SCALE_245DPS, M_SCALE_2GS);

	// client initialization
	/*client_socket_fd = client_init(argc, argv);
	if (client_socket_fd < 0) {
		return -1;
	}

	// run client, read 9DOF data, and send to server
	*/
	handle_connection(ninedof);

	return 0;
}
