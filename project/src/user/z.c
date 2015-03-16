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
int main(int argc, char **argv) 
{
    char buf[1000];
	int op = ReadBlock("/c/user/pass.txt", buf, 1024, 1);
	if(op!=0){
		Print("Buffer: %s\n", buf);
	}
    //Print("Hello Wodsfarld !!!\n");
    // int * a;
    // a = malloc(sizeof(int));
 //   	struct event_queue e_qu;
	// //e_qu = malloc(sizeof(struct event_queue));
	// struct event_interrupt e_in1;//, e_in2, e_in3;
	// e_in1.event_time = 7;
	// e_in1.kind = 1;
	// e_in1.details = NULL;
//	e_qu.insert(&e_qu, &e_in1);
	// e_in2.event_time = 5;
	// e_in2.kind = 1;
	// e_in2.details = NULL;
	// e_qu.insert(&e_qu, &e_in2);
	// e_in3.event_time = 4;
	// e_in3.kind = 1;
	// e_in3.details = NULL;
	// e_qu.insert(&e_qu,&e_in3);
	// e_qu.printQueue(&e_qu);
	// e_qu.pop(&e_qu);
	// e_qu.printQueue(&e_qu);
	// e_qu.pop(&e_qu);
	// e_qu.printQueue(&e_qu);
	// e_qu.pop(&e_qu);
	// e_qu.pop(&e_qu);
	return 1;
}
