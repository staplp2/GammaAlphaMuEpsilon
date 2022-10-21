#include "trace.h"


#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "heap.h"
#include "timer.h"
#include "fs.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//Struct for each event to be pushed into the trace
typedef struct moment_t
{
	const char* name; //event name
	char ph; //beggining or end
	int pid; //process id
	int tid; //thread id
	uint64_t time; //time of event
	//This bool is used to check if a beginning event has been matched with an ending event
	bool matched; 

} moment_t;

typedef struct trace_t
{
	heap_t* heap; //heap for memory allocation
	moment_t** events; //array of each event
	int event_count; //count of events
	int max_events; // the maxiumum allowed number of events
	const char* file_name; //name of file to be written to
	//Bool for if the trace has been started so pushes aren't put in early
	bool ready;
	
} trace_t;



trace_t* trace_create(heap_t* heap, int event_capacity)
{
	//allocate memory for the trace and events list
	trace_t* trace = heap_alloc(heap, sizeof(trace_t), 8);
	trace->heap = heap;
	trace->events = heap_alloc(heap, sizeof(moment_t*) * event_capacity, 8);
	//set starting values for a new trace
	trace->event_count = 0;
	trace->max_events = event_capacity;
	trace->ready = false;
	return trace;
}

void trace_destroy(trace_t* trace)
{
	//free all allocated memory
	for (int i = 0; i < trace->event_count; i++) {
		heap_free(trace->heap, trace->events[i]);
	}
	heap_free(trace->heap, trace->events);
	heap_free(trace->heap, trace);
}

void trace_duration_push(trace_t* trace, const char* name)
{
	//don't start pushing if the trace hasn't started or if there are already too many events
	if (trace->ready == false || trace->event_count == trace->max_events) {
		return;
	}
	//allocated memory to make new event
	moment_t* moment = heap_alloc(trace->heap, sizeof(moment_t), 8);
	//set an event for beginning, set the thread id to current thread, capture time of current event
	moment->name = name;
	moment->ph = 'B';
	moment->pid = 0;
	moment->tid = GetCurrentThreadId();
	moment->time = timer_get_ticks();
	moment->matched = false; // this event is yet to be matched by a ending event(pop)
	trace->events[trace->event_count] = moment; //add event to the trace's list
	trace->event_count++; //uptick number of events
}

void trace_duration_pop(trace_t* trace)
{
	//don't start popping if the trace hasn't started or if there are already too many events
	if (trace->ready == false || trace->event_count == trace->max_events) {
		return;
	}
	//create and allocate memory for new event
	moment_t* moment = heap_alloc(trace->heap, sizeof(moment_t), 8);
	//go backwards through event list to find unmatched begin event from the same thread
	for (int i = (trace->event_count - 1); i >= 0; i--) {
		if (trace->events[i]->tid == GetCurrentThreadId() && trace->events[i]->matched == false) {
			moment->name = trace->events[i]->name;
			trace->events[i]->matched = true;
			break;
		}
	}
	//set all the parameters for new end event
	moment->ph = 'E';
	moment->pid = 0;
	moment->tid = GetCurrentThreadId();
	moment->time = timer_get_ticks();
	moment->matched = true;
	trace->events[trace->event_count] = moment; //add event to the trace's list
	trace->event_count++; //uptick event count
}

void trace_capture_start(trace_t* trace, const char* path)
{
	//set the trae as ready so that pushes and pops can be added to the list
	trace->ready = true;
	//set name of file for later write
	trace->file_name = path;
}

void trace_capture_stop(trace_t* trace)
{
	//set trace to not capture any more pushes or pops
	trace->ready = false;
	//create a new file system for the file write
	fs_t* file_system = fs_create(trace->heap, 16);
	//This buffer is used for when writing out to the file is currently static probably should be dynamic, but I'm not too sure on how to go about that
	char buffer[10000] = "{\n\t\"displayTimeUnit\": \"ns\", \"traceEvents\" : [\n";
	//loop through each event and make sure to roganize them correctly for a chrome trace
	for (int i = 0; i < trace->event_count; i++) {
		char src[100];
		sprintf_s(src, 100, "\t\t{ \"name\":\"%s\", \"ph\" : \"%c\", \"pid\" : 0, \"tid\" : \"%d\", \"ts\" : \"%d\" }", trace->events[i]->name, trace->events[i]->ph, trace->events[i]->tid, (int)trace->events[i]->time);
		strcat_s(buffer, 10000, src);
		if (i != (trace->event_count -1)) {
			strcat_s(buffer, 10000, ",");
		}
		strcat_s(buffer, 10000, "\n");
	}
	char src[] = "\t]\n}\n";
	strcat_s(buffer, 10000, src);
	//write to the file path which was given at trace start and make sure to destroy the previously created file system
	fs_write(file_system, trace->file_name, buffer, strlen(buffer), false);
	fs_destroy(file_system);
}
