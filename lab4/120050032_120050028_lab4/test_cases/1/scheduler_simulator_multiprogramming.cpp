#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stack>
#include "event_manager.h"

using namespace std;

event_mgnr em;

struct scheduling_level {
	int level_no;
	int priority;
	int time_slice;	//either some integer or N
};

struct scheduler {
	int no_of_levels;
	vector<scheduling_level> levels_arr;
};

struct process_phase {
	int itrs;	//number of iterations
	int cpu_b;	//cpu burst time
	int io_b;	//IO burst time
};

struct process {
	int pid;
	int start_priority;
	int admission;
	vector<process_phase> phases;
};

scheduler Scheduler;
vector<process> p_list;


void handling_PROCESS_SPEC_file(){
	string line, line2;
	int pid, prior;
	int adm;
	int iter;
	int cpu_t, io_t;
	ifstream infile("PROCESS_SPEC");
	while (std::getline(infile, line))
	{
		if(line=="PROCESS"){
			process proc;
			getline(infile, line2);
			std::istringstream iss(line2);
		        if (!(iss >> pid >> prior >> adm)) { break; } // error
		    
			proc.pid = pid;
			proc.start_priority = prior;
			proc.admission = adm;

			getline(infile, line2);
			while(line2 != "END"){
				std::istringstream iss(line2);
				process_phase pp;
			        if (!(iss >> iter >> cpu_t >> io_t)) { break; } // error
			    
				pp.itrs = iter;
			    	pp.cpu_b = cpu_t;
			    	pp.io_b = io_t;
			    	(proc.phases).push_back(pp);
			    	getline(infile, line2);
			}
			p_list.push_back(proc);
			em.add_event(proc.admission,1,proc.pid, proc.start_priority);	//event type "1" represents "process admission event"

		}
	}
}

int string_to_integer(string str)
{
	int r=1,s=0,l=str.length(),i;
	for(i=l-1;i>=0;i--)
	{
		s = s + ((str[i] - '0')*r);
		r *= 10;
	}
	return s;
}

void handling_SCHEDULER_SPEC_file(){
	string line, line2;
	int level_count;
	int prior;
	int s_lvl;
	int t_slice;
	string t_slice1;
	ifstream infile("SCHEDULER_SPEC");
	while (std::getline(infile, line))
	{
		if(line=="SCHEDULER"){
			getline(infile, line2);
			std::istringstream iss(line2);
		    if (!(iss >> level_count)) { break; } // error
			
			Scheduler.no_of_levels = level_count;
			for(int i=0; i<level_count; i++){
				getline(infile, line2);
				std::istringstream iss(line2);
				if (!(iss >> s_lvl >> prior >> t_slice1)) { break; } // error
				scheduling_level scl;
				if(t_slice1 == "N")
					t_slice = 9999;
				else
					t_slice = string_to_integer(t_slice1);
				scl.level_no = s_lvl;
				scl.priority = prior;
				scl.time_slice = t_slice;
				
				Scheduler.levels_arr.push_back(scl);
			}
		}
	}
}


void printE(int pid, int etime, string msg){
	printf("PID :: %d TIME :: %d EVENT :: ", pid, etime);
	cout << msg << endl;
}

int main()
{
	
	handling_PROCESS_SPEC_file();
	handling_SCHEDULER_SPEC_file();
	//processing events

	while(!em.is_empty()){
		event next, next_conflicting;
		next = em.next_event();
		int i = 0;
		for (; i < p_list.size(); i++){
			if (next.pid == p_list[i].pid) break;
		}

		switch(next.type)
		{	
			//routine for handling process admission event
			case 1:{
				printE(next.pid, next.end_t, "CPU started");
				int end_time = next.end_t + ((p_list[i]).phases)[0].cpu_b;
				//cout << end_time << " end time\n";
				stack<event> popped_events;

				bool found = false;

				while(!em.is_empty()){
					next_conflicting = em.next_event();
					popped_events.push(next_conflicting);
					//cout << next_conflicting.pid << endl;
					if (next_conflicting.end_t >= end_time) break;
					if (next_conflicting.start_priority > next.start_priority){
						found = true;
						//cout << "here1\n";
						break;
					}
				}

				if (!found){
					em.add_event(end_time, 3, next.pid, next.start_priority);	//event type "3" represents "IO start"
					
					while(!popped_events.empty()){
						event e = popped_events.top();
						e.end_t = end_time;
						em.event_table.push(e);
						popped_events.pop();
					}

					printE(next.pid, end_time, "CPU burst completed");
				} else{
					//((p_list[i]).phases)[0]

					int j = 0;
					for (; j < p_list.size(); j++){
						if (next_conflicting.pid == p_list[j].pid) break;
					}

					if (((p_list[i]).phases)[0].itrs == 1){
						((p_list[i]).phases)[0].cpu_b -= p_list[j].admission - p_list[i].admission;
					}
					else{
						(((p_list[i]).phases)[0].itrs)--;
						process_phase new_process_phase;
						new_process_phase.itrs = 1;
						new_process_phase.cpu_b = ((p_list[i]).phases)[0].cpu_b - ( p_list[j].admission - p_list[i].admission );
						new_process_phase.io_b = ((p_list[i]).phases)[0].io_b;
						((p_list[i]).phases).insert(((p_list[i]).phases).begin(), new_process_phase);
					}

					while(!popped_events.empty()){
						event e = popped_events.top();
						e.end_t = next_conflicting.end_t;
						em.event_table.push(e);
						popped_events.pop();
					}

					em.add_event(next_conflicting.end_t , 1, next.pid, next.start_priority);	//event type "3" represents "IO start"
					printE(next.pid, next_conflicting.end_t, "Process pre-empted");
				}

				break;
			}
			//– End of time-slice
			case 2:
				break;

			//– IO start
			case 3:
			{
				printE(next.pid, next.end_t, "IO started");	
				em.add_event(next.end_t + ((p_list[i]).phases)[0].io_b , 4, next.pid, next.start_priority);	//event type "3" represents "IO start"
				break;
			}
			//– IO end
			case 4:
			{
				printE(next.pid, next.end_t, "IO burst completed");

				if ( ((p_list[i]).phases)[0].itrs == 1){
					((p_list[i]).phases).erase(((p_list[i]).phases).begin());
					if ((p_list[i]).phases.size() == 0){
						printE(next.pid, next.end_t, "Process terminated");
					}
					else{
						em.add_event(next.end_t , 1, next.pid, next.start_priority);	//event type "3" represents "IO start"
					}
				}
				else{
					(((p_list[i]).phases)[0].itrs)--;
					em.add_event(next.end_t , 1, next.pid, next.start_priority);	//event type "3" represents "IO start"
				}

				break;
			}
			//default:
			//Define routines for other required events here.

		}
	}
	return 0;
}
