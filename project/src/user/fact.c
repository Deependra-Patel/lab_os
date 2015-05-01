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

int pageWrite(int x){
    if(x == 0)
        return 1;
    char buf[1000][1000];
    buf[23][45] = 1;

    int i=1;
    // for(i=0; i<1000; i++)
    //     buf[i] = 'x';
    // int op = ReadBlock("/c/user/pass.txt", buf, 1024, 1);
    Print("%d\n\n\n", x);
    i=pageWrite(x-1);
    return i+1; 
}

int main(int argc, char **argv) 
{
 //    char buf[1000];
	// int op = ReadBlock("/c/user/pass.txt", buf, 1024, 1);
	// Print("\nHi!, Deependra\n");
	// op = ReadBlock("/c/user/pass.txt", buf, 1024, 1);

	// Print("\nDone\n");
	int x = 1000;
	int p;
	// p = factorial(x);
	int arr[x*100];
	for(p=0; p<10000; p++){
		arr[0] = 3;
		arr[100] = 5;
		arr[10000] = 6;
	}
		p = pageWrite(50);

	Print("%d", p);
	Print("here\n");
	return 1;
}
