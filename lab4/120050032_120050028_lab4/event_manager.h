#include <iostream>
#include <vector>
#include <queue>


using namespace std;

struct event{
	int end_t;	//event occurrence time 
	int type;	//event type
	int pid;	//process id
	int priority;
	bool cpuDone;
	int start_priority;	// priority of the event
	bool admitted;
};

class comp{
public:
 	int operator() ( const event& p1, const event &p2)
 	{
 		if(p1.end_t != p2.end_t)
 			return p1.end_t>p2.end_t;
 		else return p1.priority<p2.priority;
 	}
};

class event_mgnr {
	public:
		priority_queue<event, vector<event>, comp> event_table;

	//function for adding an event in the event table
	void add_event(int end_t, int type, int pid, int prior, bool adm)
	{
		event ev;
		ev.end_t = end_t;
		ev.type = type;
		ev.pid = pid;
		ev.start_priority = prior;
		ev.admitted = adm;
		event_table.push(ev);
	}

	void add_event(int end_t, int type, int pid)
	{
		event ev;
		ev.end_t = end_t;
		ev.type = type;
		ev.pid = pid;
		event_table.push(ev);
	}
	void add_event(event x){
		event_table.push(x);
	}

	//Is event table empty..?
	bool is_empty()
	{
		return event_table.empty();
	}
	
	//function for returning the top_most entry of the event table
	event next_event()
	{
		event ev = event_table.top();
		event_table.pop();
		return ev;
	}

};
