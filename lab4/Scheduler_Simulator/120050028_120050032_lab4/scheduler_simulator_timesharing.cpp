#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
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


event endOfTimeSlice(){
	return em.next_event();
}
void printE(event next, string msg){
	process cur = p_list[next.pid];
	printf("PID :: %d TIME :: %d EVENT :: ", cur.pid, next.end_t);
	cout<<msg<<endl;
}
void timeSharing(){
	event next;
	while(!em.is_empty()){
		next = em.next_event();
		process cur = p_list[next.pid];
		process_phase curPhase = cur.phases[0];
		event newE, newE2;
		switch(next.type){
			//proc admission time
			case 1:
				printE(next, "Process dispatched");
				newE.start_priority = next.start_priority;
				newE.pid = next.pid;
				newE.type = 2;

				newE2.start_priority = next.start_priority;
				newE2.pid = next.pid;

				if(curPhase.cpu_b <= Scheduler.levels_arr[0].time_slice){
					newE.end_t = next.end_t +  curPhase.cpu_b;
					newE2.type = 3;
					newE2.end_t = newE.end_t; //IO request
				}
				else {
					newE.end_t = next.end_t + Scheduler.levels_arr[0].time_slice;
					newE2.type = 1;
					newE2.end_t = newE.end_t; //again cpu request
				}

				em.add_event(newE);
				em.add_event(newE2);
				break;
			//end of time slice
			case 2:
				printE(next, "Time slice ended");				
				break;			
			//io start
			case 3:
				printE(next, "IO started");	
				newE.end_t = next.end_t + curPhase.io_b;
				newE.start_priority = next.start_priority;
				newE.pid = next.pid;
				newE.type = 4;
				em.add_event(newE);					
				break;
			//io end
			case 4:
				printE(next, "IO ended");
				curPhase.itrs--;
				if(curPhase.itrs == 0)
					cur.phases.erase(cur.phases.begin());
				if(cur.phases.size() == 0){
					printE(next, "Process Completed");
				}
				else {
					newE.end_t = next.end_t;
					newE.start_priority = next.start_priority;
					newE.pid = next.pid;
					newE.type = 1;
					em.add_event(newE);
				}
				break;
		}
	}
}

int main()
{
	
	handling_PROCESS_SPEC_file();
	handling_SCHEDULER_SPEC_file();
	//processing events
	timeSharing();
	return 0;
}
