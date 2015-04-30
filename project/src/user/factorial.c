// #include <geekos/gfs2.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <strings.h>
// #include <string.h>
// #include <fcntl.h>
// #include <errno.h>
#include <fileio.h>

#include <conio.h>
// #include <geekos/malloc.h>
#include <geekos/interrupt_queue.h>
// void insert (struct event_queue*, struct event_interrupt*);
int factorial(int x){
	if(x==1)
		return 1;
	else return x*factorial(x-1);
}

int main(int argc, char **argv) 
{
 //    char buf[1000];
	// int op = ReadBlock("/c/user/pass.txt", buf, 1024, 1);
	// Print("\nHi!, Deependra\n");
	// op = ReadBlock("/c/user/pass.txt", buf, 1024, 1);

	// Print("\nDone\n");
	int x = 10000000;
	int p;
	p = factorial(x);
	Print("%d", p);
	Print("here\n");
	return 1;
}
