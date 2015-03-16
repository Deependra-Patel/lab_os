// #include <geekos/interrupt_queue.h>
// #include <geekos/kassert.h>
// #include <geekos/defs.h>
// #include <geekos/screen.h>
// #include <geekos/int.h>
// #include <geekos/mem.h>
// #include <geekos/symbol.h>
// #include <geekos/string.h>
// #include <geekos/kthread.h>
// #include <geekos/malloc.h>
// #include <geekos/user.h>
// #include <geekos/alarm.h>
// #include <geekos/projects.h>
// #include <geekos/smp.h>
// //#include <geekos/interrupt_queue.h>
// //#define NULL 0
// //typedef int bool;
// //#define false 0;
// //#define true 1;
// void Init_node(struct node* n){
// 	//event = new event_interrupt;
// 	n->event = NULL;
// 	n->next = NULL;
// 	n->prev = NULL;
// }

// void Init_event_queue(struct event_queue* eq) {
// 	eq->first = NULL;
// }

// void insert (struct event_queue* eq, struct event_interrupt* eve) {
// 	Print("pqowel");
// 	bool found = false;
// 	struct node* temp = eq->first;
// 	if (temp == NULL) {
// 		found = true;
// 		struct node a_node;
// 		temp = &a_node;//malloc(sizeof(struct node));
// 		Init_node(temp);
// 		temp->event = eve;
// 		eq->first = temp;
// 	}
// 	else {
// 		while(temp != NULL) {
// 			if (temp->event->event_time > eve->event_time) {
// 				break;
// 			}
// 			temp = temp->next;
// 		}
// 		if (!found) {
// 			if (temp->prev != NULL) {
// 				struct node a_node;
// 				temp->prev->next = &a_node;//malloc(sizeof(struct node));
// 				Init_node(temp->prev->next);
// 				temp->prev->next->event = eve;
// 				temp->prev->next->prev = temp->prev;
// 				temp->prev->next->next = temp;
// 				temp->prev = temp->prev->next;
// 			}
// 			else {
// 				struct node a_node;
// 				temp->prev = &a_node;//malloc(sizeof(struct node));
// 				Init_node(temp->prev);
// 				temp->prev->next = temp;
// 				temp->prev->event = eve;
// 				if (temp == eq->first) {
// 					eq->first = eq->first->prev;
// 				}
// 			}
// 		}
// 	}
// }


// void pop(struct event_queue* eq) {
// 	if (eq->first != NULL) {
// 		eq->first = eq->first->next;
// 		//if (eq->first != NULL) delete(eq->first->prev);
// 	}
// 	else {
// 		eq->first = NULL;
// 	}
// }


// int getPidFirst(struct event_queue* eq){
// 	if (eq->first == NULL){
// 		return -1;
// 	}
// 	else return eq->first->event->details->event_pid;
// }

// void printQueue(struct event_queue* eq){
// 	struct node* cur = eq->first;
// 	while(cur!=NULL){
// 		Print("Pid: %d, Time%d",cur->event->details->event_pid, cur->event->event_time);
// 		cur = cur->next;
// 	}
// }