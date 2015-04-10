#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
using namespace std;

#define NUM_THREADS 5
#define MAX_SEATS 10
#define RAND_MAX 10

#define DBG 1
#define debug(x) if (DBG) {cerr << "\n" << #x << " : " << x << endl;}
#define debug2(x, y) if (DBG) {cerr << #x << " : " << x << " :: " << #y << " : " << y << endl;}
#define debug3(x, y, z) if (DBG) {cerr << #x << " : " << x << " :: " << #y << " : " << y << " :: " << #z << " : " << z << endl;}
#define rep(i,n) for(int i=0; i<(n); i++)

struct op{
  string type;
  int flight;
};

bool done[NUM_THREADS] = {false};
int count[10] = {0};//count
pthread_mutex_t mute;
pthread_mutex_t mutePass;
pthread_mutex_t finishedMute[NUM_THREADS];


pthread_cond_t passed;
pthread_cond_t finished[NUM_THREADS];

pthread_t threads[NUM_THREADS];
bool blocked[NUM_THREADS] = {true};
op *cur;

// vector<op*> operations;

void *Operate(void *threadid)
{
  long t = (long)threadid;
  pthread_mutex_lock(&finishedMute[t]);
  pthread_cond_wait(&finished[t], &finishedMute[t]);
  pthread_mutex_unlock(&finishedMute[t]); 
   
  while(true){
    // if(cur == NULL){
    //   usleep(10);
      if(done[t]){
        pthread_exit((void*) 0);
      }
      // blocked[t] = false;
      //pthread_cond_wait(&passed, &mutePass);
     // continue;
    //}
    // cout<<"upppppppp";
    pthread_mutex_lock(&mutePass);
    // blocked[t] = false;   

    op myCur = *cur;
    // cout<<"cur"<<myCur.flight<<endl;
    pthread_cond_signal(&passed);
    pthread_mutex_unlock(&mutePass);

    cout<<"Inside of thread: "<<t<<endl;
    // cout<<"For flight: "<<myCur.flight<<","<<endl;

    if(myCur.type == "BOOK"){
      pthread_mutex_lock(&mute);
      cout << "Query : BOOK " << myCur.flight << endl; 

      if(count[myCur.flight] < MAX_SEATS){
        cout << "SEAT BOOKED ON FLIGHT "  << myCur.flight << endl;
        count[myCur.flight]++;
      }
      else cout << "NO SEAT LEFT ON FLIGHT "  << myCur.flight << endl;
      
      pthread_mutex_unlock(&mute);

    }
    else if(myCur.type == "CANCEL"){
      pthread_mutex_lock(&mute);
      cout << "Query : CANCEL " << myCur.flight << endl; 
      if (count[myCur.flight] > 0){
        count[myCur.flight]--;
        cout<<"SEAT CANCELLED ON FLIGHT " << myCur.flight <<endl;
      }
      else{
        cout << "NOTHING TO CANCEL ON FLIGHT " << myCur.flight << endl;
      }
      pthread_mutex_unlock(&mute);

    }
    else if(myCur.type == "STATUS"){
      pthread_mutex_lock(&mute);
      cout << "Query : STATUS " << myCur.flight << endl; 
      cout<< MAX_SEATS - count[myCur.flight] << " seats left on flight " << myCur.flight << endl;
      pthread_mutex_unlock(&mute);

    }
    else {
      cout<<"INVALID COMMAND"<<endl;
    }
    cout << endl;
    double waitTime = rand() % 20;
    sleep(waitTime/RAND_MAX);
    blocked[t] = true;
    // debug2(t, blocked[t]);
    pthread_mutex_lock(&mutePass);
    // blocked[t] = false;   

    // op myCur = *cur;
    // cout<<"cur"<<myCur.flight<<endl;
    pthread_cond_signal(&passed);
    pthread_mutex_unlock(&mutePass);

    pthread_mutex_lock(&finishedMute[t]);
    pthread_cond_wait(&finished[t], &finishedMute[t]);
    pthread_mutex_unlock(&finishedMute[t]);


  }

   //printf("Hello World! It's me, thread #%ld!\n", tid)
}

int main(int argc, char *argv[])
{

  srand (time(NULL));
  void* status;
  pthread_mutex_init(&mute, NULL);
  pthread_mutex_init(&mutePass, NULL);

  pthread_cond_init(&passed, NULL);
  for (int i = 0; i < NUM_THREADS ; i++){
    pthread_mutex_init(&finishedMute[i], NULL);
    pthread_cond_init(&finished[i], NULL);
  }

  // for(int i =0 ; i<10; i++){
  //   count[i] = 1;
  // }
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
  rep(i, NUM_THREADS) blocked[i] = true;

   while(true){
    string temp;
    cin>>temp;
    if(temp == "END"){
      cout<<"All queries processed.\n";
      // done = true; 
      break;
    }
    int flight;

    // cout << temp;
    // debug(temp);
    cin >> flight;
    op *my = new op();
    my->flight = flight;
    my->type = temp;
    cur = my;
    // cout<<"hee"<<endl;
    for(int j=0; j<NUM_THREADS; j++){
      // debug2(j , blocked[j]);
    }
    for(int j=0; j<NUM_THREADS; j++){
        // cout << j << " j:: ";

      while(true){
        bool breakable = false;
        for (int i = 0; i < NUM_THREADS; i++)
          if (blocked[i]) breakable = true;
        if (breakable) break;
        sleep(1);
      }

      if(blocked[j]){
        // debug(j);
        blocked[j] = false;
        // cout << "passed dsd";
        // debug("dffdf");
        pthread_mutex_lock(&finishedMute[j]);
        pthread_cond_signal(&finished[j]);
        pthread_mutex_unlock(&finishedMute[j]);

        // debug("here");
        pthread_mutex_lock(&mutePass);
        pthread_cond_wait(&passed, &mutePass);
        pthread_mutex_unlock(&mutePass);
        
        // cout << endl;
        // debug("finish");
        break;
      }

    }
  //cout<<"heeeeeee";

  }
  //debug("gere");
  while(true){
    // pthread_join(threads[i], &status);
    bool x = true;
    for (int j = 0; j < NUM_THREADS; j++){
      if (blocked[j]) {
        pthread_mutex_lock(&finishedMute[j]);
        pthread_cond_signal(&finished[j]);
        pthread_mutex_unlock(&finishedMute[j]);        
        done[j] = true;
      }
      else x = false;
    }
    if (x) break;
    // debug(x);
    sleep(1);
  }
   // s Last thing that main() should do 
  //debug("gere");

  //exit(0);
  pthread_exit(NULL);
  
  pthread_mutex_destroy(&mute);
  pthread_mutex_destroy(&mutePass);
  pthread_cond_destroy(&passed);
  pthread_exit(NULL);
  return 0;
}
