
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
	int x = 10;
	int p;
	p = factorial(x);
	Print("factorial: %d\n", p);
	return 1;
}
