#include "client.h"
#include "9DOF.h"
#include <signal.h>
#include <mraa.h>
#include <sys/time.h>

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

int client_handle_connection(int client_socket_fd, NINEDOF *ninedof)
{
	int n;
	char buffer[256];
	double sec_since_epoch;

	memset(buffer, 0, 256);
	//sprintf(buffer, "time (epoch), accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, mag_x, mag_y, mag_z, temperature");
	sprintf(buffer, "time1 (epoch), accel_x1, accel_y1");

	n = write(client_socket_fd, buffer, strlen(buffer));
	if (n < 0) {
		return client_error("ERROR writing to socket");
	}

	memset(buffer, 0, 256);

	n = read(client_socket_fd, buffer, 255);
	if (n < 0) {
		return client_error("ERROR reading from socket");
	}

	printf("msg from server: %s\n", buffer);

	while (run_flag) {

		memset(buffer, 0, 256);
		
		// timestamp right before read 9DOF data
		sec_since_epoch = timestamp();
		ninedof_read(ninedof);

		// print client 9DOF readings 
		//ninedof_print(ninedof);

		// write 9DOF reading to buffer
		/*sprintf(buffer, "%10.10f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", 
			sec_since_epoch, 
			ninedof->accel_data.x, ninedof->accel_data.y, ninedof->accel_data.z,
			ninedof->gyro_data.x - ninedof->gyro_offset.x, 
  			ninedof->gyro_data.y - ninedof->gyro_offset.y, 
  			ninedof->gyro_data.z - ninedof->gyro_offset.z,
  			ninedof->mag_data.x, ninedof->mag_data.y, ninedof->mag_data.z,
  			ninedof->temperature);
		*/
		sprintf(buffer, "%10.10f,%f,%f", sec_since_epoch, ninedof->accel_data.x, ninedof->accel_data.y);
		// send 9DOF reading to server
		n = write(client_socket_fd, buffer, strlen(buffer));
		if (n < 0) {
			return client_error("ERROR writing to socket");
		}

		memset(buffer, 0, 256);

		// get response message from server
		n = read(client_socket_fd, buffer, 255);
		if (n < 0) {
			return client_error("ERROR reading from socket");
		}

		// print the message from server
		printf("msg from server: %s\n", buffer);
		usleep(10000);
	}
	close(client_socket_fd);
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
	client_socket_fd = client_init(argc, argv);
	if (client_socket_fd < 0) {
		return -1;
	}

	// run client, read 9DOF data, and send to server
	client_handle_connection(client_socket_fd, ninedof);

	return 0;
}
