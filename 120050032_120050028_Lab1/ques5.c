#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void child1(FILE* f){
      f=fopen("testabc.txt","a");            
      if(!f)
        printf("oops!\n");

      fprintf(f,"abc\n");
      fflush(f);
      fclose(f);
}

int main()
{
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = 500000;  
  long int i,j;
  FILE* f;
/*
  for (j=1;j<1000;j++){
    if ( fork() == 0 ) {
        printf("%s%i\n", "child: ", getpid() );
        child1();
        _exit(0);
      }
  }
*/
  for (j=1;j<9999999999/12;j++)
  {
    if ( fork() == 0 ) {
      printf("%s%i\n", "child: ", getpid() );
      for (i=0;i<100000000;i++)
      {  
          child1(f);
      }
           _exit(0);

    }
    if(nanosleep(&tim , &tim2) < 0 )  
    {
      printf("failed \n");
      return -1;
    }
// nanosleep(tim); //50 millis
  }
}
