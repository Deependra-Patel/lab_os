#ifndef Interrupt_h
#define Interrupt_h
#include <geekos/kthread.h>
struct event_details {
	struct Kernel_Thread * thread;
	void (*Init_event_details)();
};

struct event_interrupt {
	int event_time;
	int kind;
	struct event_details* details;
};

struct node {
	struct event_interrupt* event;
	struct node* next;
	struct node* prev;
	void (*Init_node)(struct node*);
};

struct event_queue {
	struct node* first;
	void (*Init_event_queue)(struct event_queue*);
	void (*pop)(struct event_queue*);
	struct Kernel_Thread* (*getThreadFirst)(struct event_queue*);
	void (*printQueue)(struct event_queue*);
	void (*insert) (struct event_queue*, struct event_interrupt*);
};
#endif