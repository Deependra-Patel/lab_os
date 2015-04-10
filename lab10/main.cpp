#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
using namespace std;

#define NUM_THREADS 5
#define MAX_SEATS	10

struct op{
  string type;
  int flight;
};
bool done = false;
int count[10] = {0};//count
pthread_mutex_t mute;
pthread_mutex_t mutePass;
pthread_mutex_t finishedMute[NUM_THREADS];


pthread_cond_t passed;
pthread_cond_t finished[NUM_THREADS];

pthread_t threads[NUM_THREADS];
bool blocked[NUM_THREADS] = {true};
op *cur;

vector<op*> operations;

void *Operate(void *threadid)
{
  long t = (long)threadid;
  pthread_mutex_lock(&finishedMute[t]);
  pthread_cond_wait(&finished[t], &finishedMute[t]);
  pthread_mutex_unlock(&finishedMute[t]); 
   
  while(true){
    // cout<<t<<endl;
    // if(cur == NULL){
    //   usleep(10);
      if(done){
        pthread_exit((void*) 0);
      }
      //blocked[t] = true;
      //pthread_cond_wait(&passed, &mutePass);
     // continue;
    //}
    // cout<<"upppppppp";
    pthread_mutex_lock(&mutePass);
    op myCur = *cur;
    // cout<<"cur"<<myCur.flight<<endl;
    pthread_cond_signal(&passed);
    pthread_mutex_unlock(&mutePass);

    // cout<<"Inside of thread: "<<t<<endl;
    // cout<<"For flight: "<<myCur.flight<<","<<endl;

    if(myCur.type == "BOOK"){
      pthread_mutex_lock(&mute);
      if(count[myCur.flight] < MAX_SEATS){
        cout<<"Seat booked"<<endl;
        count[myCur.flight]++;
      }
      else cout<<"Sorry no seat left.";
      pthread_mutex_unlock(&mute);

    }
    else if(myCur.type == "CANCEL"){
        cout<<"Seat CANCELLED"<<endl;
      pthread_mutex_lock(&mute);
      if (count[myCur.flight] > 0)
        count[myCur.flight]--;  
      pthread_mutex_unlock(&mute);

    }
    else if(myCur.type == "STATUS"){
      pthread_mutex_lock(&mute);

      cout<<MAX_SEATS - count[myCur.flight]<<" seats left.";
      pthread_mutex_unlock(&mute);

    }
    else {
      cout<<"errrrrr"<<endl;
    }
    
    blocked[t] = true;
    pthread_mutex_lock(&finishedMute[t]);
    pthread_cond_wait(&finished[t], &finishedMute[t]);
    pthread_mutex_unlock(&finishedMute[t]);
  }

   //printf("Hello World! It's me, thread #%ld!\n", tid)
}

int main(int argc, char *argv[])
{
  void* status;
  pthread_mutex_init(&mute, NULL);
  pthread_mutex_init(&mutePass, NULL);

  pthread_cond_init(&passed, NULL);
  for (int i = 0; i < NUM_THREADS ; i++){
    pthread_mutex_init(&finishedMute[i], NULL);
    pthread_cond_init(&finished[i], NULL);
  }

  for(int i =0 ; i<10; i++){
    count[i] = 1;
  }
  int rc;
  int t;
  for(t=0; t<NUM_THREADS; t++){
    printf("In main: creating thread %i\n", t);
    rc = pthread_create(&threads[t], NULL, Operate, (void *) t);
    if (rc){
      printf("ERROR; return code from pthread_create() is %i\n", rc);
      exit(-1);
    }
  }

   while(true){
    string temp;
    cin>>temp;
    if(temp == "END"){
      cout<<"All queries processed.";
      done = true; 
      break;
    }
    int flight;
    cout << temp;
    cin >> flight;
    op *my = new op();
    my->flight = flight;
    my->type = temp;
    cur = my;
    // cout<<"hee"<<endl;
    for(int j=0; j<NUM_THREADS; j++){
      if(blocked[j]){
        // cout << "passed dsd";
        pthread_mutex_lock(&finishedMute[j]);
        pthread_cond_signal(&finished[j]);
        pthread_mutex_unlock(&finishedMute[j]);

        pthread_mutex_lock(&mutePass);
        pthread_cond_wait(&passed, &mutePass);
        pthread_mutex_unlock(&mutePass);
        // cout << "passed";
        cout << endl;
        break;
      }
    }
  }

  for(int i=0; i<NUM_THREADS; i++){
    pthread_join(threads[i], &status);
  }    
   // s Last thing that main() should do 
  
  pthread_mutex_destroy(&mute);
  pthread_mutex_destroy(&mutePass);
  pthread_cond_destroy(&passed);
  pthread_exit(NULL);
}
