#include <stdio.h>
# include <math.h>
//# include <queue>
# include <time.h>
# include <stdlib.h>
# include <string.h>
# include "ssq.h"
#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2
#define MAX_HEAP_SIZE 5000
/* 5. random number generator */
double uniform(double *);
double expon(double *, float);
double cumulativedelay = 0;
double cumulativeaverage = 0;
double seed; //set the seed only once, at the start.

float rho;
int max_no_of_customers_allowed;
char random_seed_file[30];
int output_option;
double clocker;
int count;
//int done = 0;
double arrivaltime[2500];
double endofservicetime[2500];
double meaninterarrival;
double meanservice;
double avgcustomers;
int serverstatus;
double servicetime[2500];
double totalservicetime;
//arrivaltime[0] = 0.0;
int countendschedules =0;
double delay;
double arrival_curr;
double prev_end_service;
double totaldelay;
double cumultaivetime;
double averagenumber;
//represents arrival and end of service
struct event{
    int index;
    char* eventtype;
    double timestamp;
};
//acts as calender to store next arrival/end of service event scheduled.
//has atmost two customers at a time
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
    if(par->timestamp > cur->timestamp) {
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
    if (ev1->timestamp < ev2->timestamp) {
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
    if (cur->timestamp > min_child->timestamp) {
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

struct queue* head = NULL;

void printUsage() {
    printf("two few arguments!!usage: ./ssq rho max_no_of_cust rand_seed_filename output_option \n");
    printf("example: ./ssq 0.9 2000 rand_seed.txt 2 \n");
}

void makeserverbusy(){
  serverstatus = 1;
}

void makeserveridle(){
  serverstatus = 0;
}

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

void scheduleevent(char* eventtype,double clocker, int index,struct heap* hp){
    char* event = eventtype;
    double time;
    struct event* newevent = (struct event*)malloc(sizeof(struct event));
    if(strcmp(event,"A") == 0){
      time = clocker + expon(&seed,meaninterarrival);
      arrivaltime[index] = time;
    }
    else if(strcmp(event,"E") == 0){
      time = clocker + expon(&seed, meanservice);
        servicetime[index] = time - clocker;
    }
    else{
      printf("event is neither arrival nor end of service!! \n");
    }
    newevent->index = index;
    newevent->eventtype = event;
    newevent->timestamp = time;
    putevent(hp, newevent);
}
void handlearrival(struct event* event,double clocker, int i,struct heap* hp){
    scheduleevent("A",clocker,i,hp);
    if(serverstatus == 1){
       enqueue(event,&head);  
    }
    else{
      makeserverbusy();
      scheduleevent("E",clocker,event->index,hp);
    }
}
void handleendofservice(struct event* e, double clocker,struct heap* hp){
   endofservicetime[e->index] = e->timestamp;
   if(head == NULL){
     makeserveridle();
   }
   else{
     struct event* nextevent = dequeue(&head);
     scheduleevent("E",clocker,nextevent->index,hp);
   }
}
void simulation(double seed,struct heap* hp){
  clocker = 0;
  count = 0;
  avgcustomers = 0;
  serverstatus = 0;
  head = NULL;
  totalservicetime=0;
  delay = 0;
  arrival_curr=0;
  prev_end_service=0;
  totaldelay = 0;
  cumultaivetime =0;
  averagenumber =0;
  int i=1;
  makeserveridle();
  scheduleevent("A",clocker,i,hp);
  while(count < max_no_of_customers_allowed){
        struct event* e = getevent(hp);
        clocker = e->timestamp;
        if(strcmp(e->eventtype,"A")==0){
            i++;
            handlearrival(e,clocker,i,hp);
        }
        else if(strcmp(e->eventtype,"E")==0){
            count++;
            handleendofservice(e,clocker,hp);
        }
        else{
            printf("Event fetched from Calender is neither Arrival nor EOS!!\n");
        }
   }
  for(int i=2;i<=max_no_of_customers_allowed;i++){
    if(arrivaltime[i] < endofservicetime[i-1]){
      delay = endofservicetime[i-1] - arrivaltime[i];
    }
    else{
      delay = 0;
    }
    totaldelay = totaldelay+ delay;
  }
  
  
  for(int i = 1 ; i<=(max_no_of_customers_allowed);i++){
          totalservicetime = totalservicetime+servicetime[i];
  }
  totalservicetime = totalservicetime/(max_no_of_customers_allowed);
  totaldelay = ((totaldelay / max_no_of_customers_allowed) + totalservicetime);
  printf("Average waiting time in system = %.3f \n", totaldelay);
  memset(arrivaltime, 0, sizeof(arrivaltime));
  memset(endofservicetime, 0, sizeof(endofservicetime));
  memset(servicetime, 0, sizeof(servicetime));
  cumulativedelay = cumulativedelay + totaldelay;
}

int main(int argc,char* argv[]){

    // read the arguments from command line
    if (argc == 1) {
        printUsage();
        exit(0);
    } else if (argc == 2) {
        rho = atof(argv[1]);
        max_no_of_customers_allowed = 2000;
        strcpy(random_seed_file, "rand_seed.txt");
        output_option = 1;
    } else if (argc == 3) {
        rho = atof(argv[1]);
        max_no_of_customers_allowed = atoi(argv[2]);
        strcpy(random_seed_file, "rand_seed.txt");
        output_option = 1;
    } else if (argc == 4) {
        rho = atof(argv[1]);
        max_no_of_customers_allowed = atoi(argv[2]);
        strcpy(random_seed_file, argv[3]);
        output_option = 1;
    } else if (argc == 5) {
        rho = atof(argv[1]);
        max_no_of_customers_allowed = atoi(argv[2]);
        strcpy(random_seed_file, argv[3]);
        output_option = atoi(argv[4]);
    }

    FILE *randfile;
    randfile = fopen(random_seed_file,"r");
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    char* stopstring;
    meaninterarrival = 1/0.25;
    meanservice = 2;
    struct heap* hp = (struct heap*)malloc(sizeof(struct heap));
    hp->size = 0;
    hp->arr = (struct event**)malloc(100*sizeof(struct event*));
     if(randfile != NULL){
       while((nread = getline(&line,&len,randfile)!= -1)){
          seed = strtod(line, &stopstring);
          //printf("%f \n", seed);
          simulation(seed,hp);
      }
      fclose (randfile); 
    }
    else{
        printf("Unable to open file \n");
        exit(0);
    }
    printf("Single-server queueing system \n");
    printf("Mean interarrival time %.3f",1/rho);
    printf(" minutes\n");
    printf("Mean service time %.3f",meanservice);
    printf(" minutes\n");
    printf("Number of customer %d \n",max_no_of_customers_allowed);
    printf("Value of rho %.2f \n",rho);
    if (output_option == 1) {
      printf("Average waiting time in system= %.3f",cumulativedelay/10);
      printf(" minutes\n");
    }


    return 0;
}
