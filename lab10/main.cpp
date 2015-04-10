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
void *Operate(void *myOp)
{
  op cur = *(op*)myOp;


   // long tid;
   // tid = (long)threadid;
    for(int i =0 ; i<10; i++)
     {
      cout<<count[i]<<endl;
     } 
  //count[tid] =  1111+tid;

   //printf("Hello World! It's me, thread #%ld!\n", tid);
   pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  vector<op*> operations;

  while(true){
    string temp;
    cin>>temp;
    if(temp == "END")
      break;
    int flight;
    cin>>flight;
    op *my = new op();
    my->flight = flight;
    my->type = temp;
    operations.push_back(my);
  }
  for(int i =0 ; i<10; i++){
    count[i] = 1;
  }
  int rc;
  int t;
  pthread_t threads[NUM_THREADS];
    for(t=0;t<NUM_THREADS;t++){
      printf("In main: creating thread %i\n", t);
      op *myOp = operations[0];
      operations.erase(operations.begin());
      rc = pthread_create(&threads[t], NULL, Operate, (op*) myOp);
      if (rc){
        printf("ERROR; return code from pthread_create() is %i\n", rc);
        exit(-1);
      }
    }
   /* Last thing that main() should do */
  pthread_exit(NULL);
}
