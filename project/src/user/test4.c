
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
	int x = 2000;
	int p;

	char ch[100][100];
	for(p = 0; p<1000; p++){
		ch[23][78] = p*p;
		ch[p][45] = ch[23][78];
 	}
 	Print("done1 \n");

 	p = factorial(1000);
 	ch[23][78] = 34;
	Print("factorial: %d\n", p);
	Print("done \n");
	return 1;
}
