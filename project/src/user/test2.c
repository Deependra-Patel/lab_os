
#include <fileio.h>

#include <conio.h>
// #include <geekos/malloc.h>
#include <geekos/interrupt_queue.h>
// void insert (struct event_queue*, struct event_interrupt*);
int func(int x){
	int i;
	i = 1;
	if(x == 0)
		return 1;
	func(x-1);
	return i;
}

int main(int argc, char **argv) 
{
	int p;
	char c[100];
	for(p = 0; p<10; p++){
		c[2]= 65;
		c[5] = 54;
	}
	p = func(1000);
	Print("Output: %d\n", p);
	return 1;
}
