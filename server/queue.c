

#include "queue.h"

void enqueue(struct Queue** Q, float data)
{
	struct qnode* new=malloc(sizeof(struct qnode));
	new->data=data;
	new->next=NULL;
	struct Queue* t;
	t=*(Q);
	t->length++;

	if(t->front==NULL)
	{
		t->front=new;
		t->rear=new;
		return;
	}
	struct qnode* temp=t->rear;
	temp->next=new;
	t->rear=new;
}
int dequeue(struct Queue** q)
{
	struct Queue *t= (*q);
	t->length--;
	if(t->rear== t->front)
	{struct qnode* temp=t->front;
		int i=temp->data;
		t->front=NULL;
		t->rear=NULL;
		free(temp);
		return i;
	}
	struct qnode* temp=t->front;
	t->front=t->front->next;
	int i=temp->data;
	free(temp);
	return i;

}

void print(struct qnode* q)
{
	struct qnode* temp=q;
	while(temp!=NULL)
	{
		printf("%f\n",temp->data );
		temp=temp->next;
	}
}
void get_data(struct qnode* q,float arr[])
{
    int i=MAX_QUEUELENGTH - 1;
    struct qnode* temp=q;
    while(temp!=NULL)
    {
	arr[i]=temp->data;
	temp=temp->next;
	i--;
    }
}
/*
int main()
{
	struct Queue* q=malloc(sizeof(struct Queue));
	q->front=NULL;
	q->rear=NULL;
	q->length=0;
	q->maxlength=MAX_QUEUELENGTH;
	enqueue(&q,50);
	enqueue(&q,40);
	enqueue(&q,20);
	print(q->front);
	printf("%d",dequeue(&q));
	print(q->front);



}
*/
