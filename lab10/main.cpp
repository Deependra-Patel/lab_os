#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

#define NUM_THREADS	5

struct op{
  string type;
  int flight;
};

int count[10];//count
pthread_mutex_t mute;
pthread_mutex_t mutePass;

pthread_cond_t passed;

pthread_t threads[NUM_THREADS];
bool blocked[NUM_THREADS];
op *cur;

vector<op*> operations;

void *Operate(void *threadid)
{
  long t = (long)threadid;
  while(true){
    pthread_mutex_lock(&mutePass);
    op myCur = *cur;
    pthread_cond_signal(&passed);
    pthread_mutex_lock(&mutePass);

    cout<<"Inside of thread: "<<t<<endl;
    cout<<"For flight: "<<myCur.flight<<","<<endl;

    if(myCur.type == "BOOK"){
      if(count[myCur.flight]>0)
        count[myCur.flight]--;
      else cout<<"Sorry no seat left.";
    }
    else if(myCur.type == "CANCEL"){
      count[myCur.flight]++;
    }
    else if(myCur.type == "STATUS"){
      cout<<count[myCur.flight]<<" seats left.";
    }
    else {
      cout<<"errrrrr"<<endl;
    }
    blocked[t] = true;
    // pthread_block() 
  }

   //printf("Hello World! It's me, thread #%ld!\n", tid);
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  void* status;
  pthread_mutex_init(&mute, NULL);
  pthread_mutex_init(&mutePass, NULL);

  pthread_cond_init(&passed, NULL);
 

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
/*
   while(true){
    string temp;
    cin>>temp;
    if(temp == "END"){
      cout<<"All queries processed.";
      break;
    }
    int flight;
    cin >> flight;
    op *my = new op();
    my->flight = flight;
    my->type = temp;

    for(int j=0; j<NUM_THREADS; j++){
      if(blocked[j]){
        pthread_mutex_lock(&mutePass);
        pthread_cond_wait(&passed, &mutePass);
        pthread_mutex_unlock(&mutePass);
        break;
      }
    }
  }

  for(int i=0; i<NUM_THREADS; i++){
    pthread_join(threads[i], &status);
  }    
   // s Last thing that main() should do 
  */
  pthread_mutex_destroy(&mute);
  pthread_mutex_destroy(&mutePass);
  pthread_cond_destroy(&passed );
  pthread_exit(NULL);
}
