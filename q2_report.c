/* External definitions for single-server queueing system */
/* 5. Random Number Generator */
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

double uniform(double *);
double expon(double *, float);
double servicetime[4500];
double endofservicetime[4500];
double arrivaltime[4500];
double delay;
double prev_end_service;
double totaldelay;
double seed; //set the seed only once, at the start.
double mean_service_time;
double mean_interarrival_time;
int max_no_of_customers_allowed =4000;
double cumulativedelay = 0;
int count;
double clocker;
int* server;
double totalservicetime;
void printUsage() {
    printf("Usage: ./q2 lambda1 lambda2 mu seed \n");
    printf("Example: ./q2 0.33 0.25 0.5 123457 \n");
}

struct event {
    int index;
    char* eventtype;
    double timestamp;
    int server;
    time_t entertime;
    //int priority;
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
         par->entertime > cur->entertime)) {
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
    if (ev1->timestamp < ev2->timestamp ||
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

struct queue* head1 = NULL;
struct queue* createnode(struct event* event){
  struct queue* newnode = (struct queue*)malloc(sizeof(struct queue));
  newnode->event= event;
  newnode->next = NULL;
  return newnode;
}

void enqueue1(struct event* event, struct queue** head1){
  struct queue* headnode = *head1;
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
 *head1 = headnode;
}

struct event* dequeue1(struct queue** head1){
  struct event* currevent = (struct event*)malloc(sizeof(struct event));
  struct queue* currnode = (*head1);
  if(currnode == NULL){
    printf("Queue is empty!!\n");
  }
  currevent =  currnode->event;
  struct queue* temp = currnode->next;
  *head1 = temp;
  return currevent;
}

void scheduleevent(char* eventtype,double clocker, int index,struct heap* hp,int servernum){
  char* event = eventtype;
  double time;
  struct event* newevent = (struct event*)malloc(sizeof(struct event));
  if(strcmp(event,"A") == 0){
    time = clocker + expon(&seed,mean_interarrival_time);
    arrivaltime[index] = time;
 //   printf("arrival time %f of %d \n",arrivaltime[index],index);
  }
  else if(strcmp(event,"E") == 0){
    time = clocker+ expon(&seed,mean_service_time);
    servicetime[index] = time - clocker;
  //  printf("service time %f of %d \n",servicetime[index],index);
  }
  else{
    printf("event is neither arrival nor end of service!! \n");
  }
  newevent->index = index;
  newevent->eventtype = event;
  newevent->timestamp = time;
  newevent->server = servernum;
  newevent->entertime = clock();
  putevent(hp,newevent);
}

void handlearrival(struct event* event,double clocker,int i, struct heap* hp){
  scheduleevent("A",clocker,i,hp,2);
  if(server[0] ==0){
    server[0] = 1;
    scheduleevent("E",clocker,event->index,hp,0);
  }
  else if(server[0] == 1){
    if(server[1] ==0){
      server[1] = 1;
      scheduleevent("E",clocker,event->index,hp,1);
    }
    else if(server[1] == 1){
      enqueue1(event,&head1);
    }
  }
}

void handleendofservice0(struct event* e, double clocker,struct heap* hp){
  endofservicetime[e->index] = e->timestamp;
  if(head1 == NULL){
    server[0] = 0;
  }
  else{
    struct event* nextevent = dequeue1(&head1);
    scheduleevent("E",clocker,nextevent->index,hp,0);
  }
}

void handleendofservice1(struct event* e, double clocker,struct heap* hp){
  endofservicetime[e->index] = e->timestamp;
  if(head1 == NULL){
    server[1] = 0;
  }
  else{
    struct event* nextevent = dequeue1(&head1);
    scheduleevent("E",clocker,nextevent->index,hp,1);
  }
}

void simulation(double seed,double mean_service_time,double mean_interarrival_time,struct heap* hp){
  clocker = 0;
  count = 0;
  int i =1;
  scheduleevent("A",clocker,i,hp,2);
  while(count < max_no_of_customers_allowed){
    struct event* e = getevent(hp);
    clocker = e->timestamp;
    if(strcmp(e->eventtype,"A")==0){
      i++;
      handlearrival(e,clocker,i,hp);
    }
    else if(strcmp(e->eventtype,"E")==0){
      if(count <11){
        printf("Customer exit time for event %d is %f \n",e->index,e->timestamp);
      }
      if(e->server == 0){
        count++;
        handleendofservice0(e,clocker,hp);
      }
      else if(e->server == 1){
        count++;
        handleendofservice1(e,clocker,hp);
      }
      else{
        printf("please check server num!! \n");
      }
    }
    else{
      printf("Event fetched from Calender is neither Arrival nor EOS!!\n");
    }
  }
  totaldelay =0;
  for(int i=2;i<=max_no_of_customers_allowed;i++){
    if(arrivaltime[i] < endofservicetime[i-1]){
      delay = endofservicetime[i-1] - arrivaltime[i];
    }
    else{
      delay = 0;
    }
    totaldelay = totaldelay+ delay;
  }
  for(int i = 1;i<= (max_no_of_customers_allowed);i++){
     totalservicetime = totalservicetime+servicetime[i];
  }
  totalservicetime = totalservicetime/(max_no_of_customers_allowed);
  totaldelay = ((totaldelay / max_no_of_customers_allowed) + totalservicetime);
  printf("Average waiting time in system= %.3f \n",totaldelay);
  cumulativedelay = cumulativedelay+ totaldelay;
}
int main(int argc,char* argv[]) {

    // read the arguments from command line
    if (argc < 5) {
        printUsage();
        exit(1);
    } else{
        double lambda1 = atof(argv[1]);
        double lambda2 = atof(argv[2]);
        double mu = atof(argv[3]);
        seed = atof(argv[4]);
        char random_seed_file[30];
        strcpy(random_seed_file, "rand_seed.txt");
        FILE *randfile;
        randfile = fopen(random_seed_file,"r");
        char* line = NULL;
        size_t len = 0;
        ssize_t nread;
        char* stopstring;
        if(randfile != NULL){
          int i = 0;
          while((nread = getline(&line,&len,randfile)!= -1)){
            seed = strtod(line, &stopstring);
            //printf("seed : %f \n",seed);
            struct heap* hp = (struct heap*)malloc(sizeof(struct heap));
            hp->size = 0;
            hp->arr = (struct event**)malloc(1000*sizeof(struct event*));
            server = (int *)malloc(2 * sizeof(int));
            server[0] = 0;
            server[1] = 0;
            mean_service_time = 1/mu;
            mean_interarrival_time = 1/(lambda1+lambda2);
            for(int i=0;i<4500;i++){
              servicetime[i] = 0;
              arrivaltime[i] = 0;
              endofservicetime[i] = 0;
            }
            totalservicetime =0;
            totaldelay = 0;
            head1 = NULL;
            simulation(seed,mean_service_time,mean_interarrival_time,hp);
          }
          fclose(randfile);
          printf("total wait time on avg %f \n", cumulativedelay/10);
        }
        // TODO: Simulation Process

        // TODO: Output Statistics
      
    }

    return 0;
}
