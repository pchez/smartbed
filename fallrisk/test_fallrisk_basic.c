#include <stdio.h>
#include "queue.h"
#include <floatfann.h>

int main(int argc, char **argv)
{
    struct Queue* fall_queue = malloc(sizeof(struct Queue));
    fann_type *output;
    fann_type input[3];
    fann_type max;
    struct fann *ann;
    int patient_risk;

    //if (argc!=3) {
//	printf("invalid number of arguments\n");
//	return -1;
  //  }

    fall_queue->front = NULL;
    fall_queue->rear = NULL;
    fall_queue->length = 0;
    fall_queue->maxlength = MAX_QUEUELENGTH;

    char *pEnd;
    float q0, q1, q2;

    enqueue(&fall_queue, 0.0);
    enqueue(&fall_queue, 0.5);
    enqueue(&fall_queue, 1.0);

 //   printf("%f", fall_queue->front);
 //   dequeue(&fall_queue);
  //  printf("%f", fall_queue->front);
  //
    printf("queue:\n");
    print(fall_queue->front);
    ann = fann_create_from_file("fallrisk.net");
    get_data(fall_queue->front, input);
    int i;

    printf("\narray input into neural network\n");
    for (i=0; i<3; i++) {
	printf("%f\n", input[i]);
    }	
    output = fann_run(ann, input);
    max = output[0];
    patient_risk = 0; //0 is safe, 1 is monitor, 2 is alarm
    for (i=1; i<3; i++) {
	if (output[i] > max) {
	    max = output[i];
	    patient_risk = i;
	}
    }	
    printf("safe(0) monitor(1) alarm(2): %d\n", patient_risk);
}
