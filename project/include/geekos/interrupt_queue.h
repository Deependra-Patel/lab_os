#ifndef Interrupt_h
#define Interrupt_h
#include <geekos/kthread.h>
#include <geekos/list.h>

DEFINE_LIST(Node_Queue, node);


struct event_details {
	struct Kernel_Thread * thread;
	void (*Init_event_details)();
};

struct node {
	int event_time;
	int kind;
	struct event_details* details;
	DEFINE_LINK(Node_Queue, node);
};

// struct node {
// 	struct event_interrupt* event;
// 	struct node* next;
// 	struct node* prev;
// 	void (*Init_node)(struct node*);
//     DEFINE_LINK(Node_Queue, node);
// };

// struct event_queue {
// 	struct node* first;
// 	void (*Init_event_queue)(struct event_queue*);
// 	void (*pop)(struct event_queue*);
// 	struct Kernel_Thread* (*getThreadFirst)(struct event_queue*);
// 	void (*printQueue)(struct event_queue*);
// 	void (*insert) (struct event_queue*, struct event_interrupt*);
// };
#endif

#ifdef GEEKOS
IMPLEMENT_LIST(Node_Queue, node);

static __inline__ void Enqueue_Node(struct Node_Queue *queue,
                                      struct node *n) {
    Add_To_Back_Of_Node_Queue(queue, n);
}

static __inline__ void Remove_Node(struct Node_Queue *queue,
                                      struct node *n) {
    Remove_From_Node_Queue(queue, n);
}
#endif