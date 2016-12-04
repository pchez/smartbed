#include <stdio.h>
#include <stdlib.h>

#define MAX_QUEUELENGTH 6
struct Queue
{
	//int data;
	struct qnode* front;
	struct qnode* rear;
	int length;
	int maxlength;
};

struct qnode
{
	float data;
	struct qnode* next;
};
void enqueue(struct Queue** Q, float data);
int dequeue(struct Queue** q);
void print(struct qnode* q);
void get_data(struct qnode* q,float arr[]);

