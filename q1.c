# include <stdio.h>
# include <math.h>
# include <time.h>
# include <stdlib.h>
# include <string.h>
# include "ssq.h"
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2
#define MAX_HEAP_SIZE 5000
/* 5. Random Number Generator */
double uniform(double *);
double expon(double *, float);
double seed; //set the seed only once, at the start.
double mean_service_time0;
double mean_service_time1;
double mean_arrival_time;
int successprob;
int num_of_queue;
int* server;
double clocker;
int count;
void printUsage() {
    printf("Usage: ./q1 lambda mu0 mu1 prob0 seed \n");
    printf("Example: ./q1 1.0 6.0 5.0 0.2 123457 \n");
}

struct event {
    int index;
    char* eventtype;
    double timestamp;
    int sequence;
    time_t entertime;
    int outsideworld;
};

struct heap {
    int size ;
    struct event** arr;
};

void swap(struct event* ev1, struct event* ev2) {
  struct event tmp; 
  tmp = *ev2;
  *ev2 = *ev1;
  *ev1 = tmp;
}

void bubble_up(struct heap* hp) {
  if (hp->size <=0) {
    return;
  }
  int cur_index = hp->size-1;
  /* invariant: cur_index to size-1 all obey heap property */
  while(cur_index > 0) {
    int par_index = PARENT(cur_index);
    struct event* cur = hp->arr[cur_index];
    struct event* par = hp->arr[par_index];
    if(par->timestamp > cur->timestamp ||
       (par->timestamp == cur->timestamp &&
         par->entertime > cur->entertime)) 
         {
      swap(par,cur);
    }
    else {
      break;
    }
    cur_index = par_index;
  }
}

void putevent(struct heap *hp, struct event* ev) {
    hp->arr[hp->size++] = ev;
    bubble_up(hp);
}

int min(struct heap* hp, int i, int j) {
  if (i >= hp->size || j>= hp->size) {
    printf("Something wong!\n");
    exit(0);
  }
  struct event* ev1 = hp->arr[i];
  struct event* ev2 = hp->arr[j];
    if (ev1->timestamp < ev2->timestamp||
        (ev1->timestamp == ev2->timestamp &&
         ev1->entertime < ev2->entertime)) {
      return i;
    }
    else {
      return j;
    }
}

void bubble_down(struct heap* hp) {
  if (hp->size <=0) {
    return;
  }
  int cur_index = 0;
  /* invariant: 0 to cur_index all obey heap property */
  while(LCHILD(cur_index) < hp->size) {
    int lc_index = LCHILD(cur_index);
    int rc_index = RCHILD(cur_index);
    int mc_index;
    if (rc_index < hp->size) {
      mc_index = min(hp, lc_index, rc_index);
    }
    else {
      mc_index = lc_index;
    }
    struct event* min_child = hp->arr[mc_index];
    struct event* cur = hp->arr[cur_index];
    if (cur->timestamp > min_child->timestamp ||
        (cur->timestamp == min_child->timestamp &&
        cur->entertime > min_child->entertime)) {
      swap(cur,min_child);
    }
    cur_index = mc_index;
  }
}

struct event* getevent(struct heap* hp) {
    if (hp->size <= 0) {
      return NULL;
    }
    struct event* min_event = hp->arr[0];
    hp->arr[0] = hp->arr[hp->size-1];
    hp->size--;
    bubble_down(hp);
    return min_event;
}

struct queue{
  struct event* event;
  struct queue* next;
};

struct queue* *qwait;
struct queue* createnode(struct event* event){
  struct queue* newnode = (struct queue*)malloc(sizeof(struct queue));
  newnode->event= event;
  newnode->next = NULL;
  return newnode;
}

void enqueue(struct event* event, struct queue** head){
  struct queue* headnode = *head;
  if(headnode == NULL){
    headnode = createnode(event);
  }
  else{
    struct queue* currnode = headnode;
    struct queue* prevnode = headnode;
    while(currnode != NULL){
      prevnode = currnode;
      currnode = currnode->next;
    }
    currnode = createnode(event);
    prevnode->next = currnode;
  }
 *head = headnode;
}

struct event* dequeue(struct queue** head){
  struct event* currevent = (struct event*)malloc(sizeof(struct event));
  struct queue* currnode = (*head);
  if(currnode == NULL){
    printf("Queue is empty!!\n");
  }
  currevent =  currnode->event;
  struct queue* temp = currnode->next;
  *head = temp;
  return currevent;
}

double getservice(int sequence){
  if(sequence ==0){
    return mean_service_time0; 
  }
  else if(sequence ==1){
    return mean_service_time1;
  }
  else{
    printf("Eevent sequence is neither 0 nor 1! \n");
    exit(0);
  }
}

//we keep resetting the sequnce number to 0 once the event gets a seq
//num of 2. If the event is generated from outside, we flag it as 1.
void scheduleevent(char* eventtype,double clocker,int sequence,int index,double meantime,struct heap* hp){
  if(sequence == num_of_queue){
    sequence =0;
  }
  else{
    sequence= sequence;
  }
  char* event = eventtype;
  double time;
  int outsideflag;
  struct event* newevent = (struct event*)malloc(sizeof(struct event));
  if(strcmp(event,"A") == 0){
  //printf("schedule eve index %d,sequence %d \n",index,sequence);
    if(meantime == 0){
      time = clocker;
      outsideflag = 0;
    }
    else{
      time = clocker + expon(&seed,meantime);
      outsideflag = 1;
    }
  }
  else if(strcmp(event,"E") == 0){
    time = clocker + expon(&seed,meantime);
    outsideflag = 0;
  }
  else{
    printf("event is neither arrival nor end of service!! \n");
  }
  newevent->index = index;
  newevent->eventtype = event;
  newevent->timestamp = time;
  newevent->sequence = sequence;
  newevent->entertime = clock();
  newevent->outsideworld = outsideflag;
  putevent(hp,newevent);
}
//if we are handling and event generated using lambda, we schedue a
//new event for arrival or else we do the usual enqueue/eos thingy
void handlearrival(struct event* event,double clocker,struct heap* hp){
  //printf("arrival event details: index %d,timestamp %f \n",event->index,event->timestamp);
  if(event->outsideworld == 1){
    scheduleevent("A",clocker,0,event->index + 1,mean_arrival_time,hp);
  }
  if(event->sequence < num_of_queue){
    if(server[event->sequence] == 0){
      server[event->sequence] = 1;
      double meantime = getservice(event->sequence);
      scheduleevent("E",clocker,event->sequence,event->index,meantime,hp);
    }
    else{
      enqueue(event,&qwait[event->sequence]);
    }
  }
  else{
    printf("check queue sequence \n");
  }
}
//on probability basis we either forward the customer to the next
//queue or we exit it from the system. if queue is not empty we get
//the next event and schedule its eos else we make the server idle.
void handleendofservice(struct event* event,double clocker,struct heap* hp){
 // if(event->sequence == 1){
    //printf("event index is %d ,time is %f,sequence is %d \n",event->index,event->timestamp,event->sequence);
 // }
  if(event->sequence == 0){
    //srand(time(0));
    double forward = uniform(&seed);
    //int forward = (rand() % 100) < 80;
    //printf("forward %f\n",roundf(forward *10)/10);
    if(forward >0.2){
      scheduleevent("A",clocker,event->sequence+1,event->index,0,hp);
    }
    else if(forward <= 0.2){
      printf("event index is %d ,time is %f,clocker is %f \n",event->index,event->timestamp,clocker);
    }
  }
  if(qwait[event->sequence] != NULL){
    struct event* nextevent = dequeue(&qwait[event->sequence]);
    double meantime = getservice(nextevent->sequence);
    scheduleevent("E",clocker,nextevent->sequence,nextevent->index,meantime,hp);
  }
  else{
    server[event->sequence] = 0;
  }
}

void simulation(double seed,double mean_service_time0,double mean_service_time1,
    double mean_arrival_time,int successprob,int num_of_queue,struct heap* hp){
  clocker = 0;
  count = 0;
  scheduleevent("A",clocker,0,1,mean_arrival_time,hp);
  while(count <= 20){
    struct event* e = getevent(hp);
    clocker = e->timestamp;
    if(e->outsideworld == 1){
      count++;
    }
    if(strcmp(e->eventtype,"A")==0){
      handlearrival(e,clocker,hp);
    }
    else if(strcmp(e->eventtype,"E")==0){
      handleendofservice(e,clocker,hp);
   //   count++;
    }
    else{
      printf("Event fetched from Calender is neither Arrival nor EOS!!\n");
    }
  }
}
int main(int argc,char* argv[]) {

    // read the arguments from command line
    if (argc < 6) {
        printUsage();
        exit(1);
    } else{
        // argv[0] is program name
        double lambda = atof(argv[1]);
        double mu0 = atof(argv[2]);
        double mu1 = atof(argv[3]);
        double prob0 = atof(argv[4]);
        seed = atof(argv[5]);
        printf("%f \n",seed);
        mean_service_time0 = 1/mu0;
        mean_service_time1 = 1/mu1;
        mean_arrival_time = 1/lambda;
        successprob = 1-prob0;
        num_of_queue = 2;
        struct heap* hp = (struct heap*)malloc(sizeof(struct heap));
        hp->size = 0;
        hp->arr = (struct event**)malloc(100*sizeof(struct event*));
        qwait = (struct queue**)malloc(num_of_queue*sizeof(struct queue*));
        server = (int *)malloc(num_of_queue * sizeof(int));
        for(int i=0;i<num_of_queue;i++){
          server[i] = 0;
          qwait[i] = NULL;
        }
        simulation(seed,mean_service_time0,mean_service_time1,mean_arrival_time,successprob,num_of_queue,hp);
        // TODO: Initialize arrival and service means, statistics variables

        // TODO: Simulation Process

        // TODO: Output Statistics

    }

    return 0;
}
