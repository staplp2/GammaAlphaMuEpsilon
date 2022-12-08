#pragma once

typedef struct heap_t heap_t;

typedef struct trace_t trace_t;

/// Creates a CPU performance tracing system.
/// @param heap Heap for allocation of memory for the trace object.
/// @param event_capacity The maximum number of durations that can be traced.
/// @return The created trace object.
trace_t* trace_create(heap_t* heap, int event_capacity);

/// Destroys a CPU performance tracing system.
/// @param trace Trace object to be destroyed.
void trace_destroy(trace_t* trace);

/// Begin tracing a named duration on the current thread.
/// It is okay to nest multiple durations at once.
/// @param trace Trace object for which to push named duration onto.
/// @param name Name of object which will be pushed onto the given trace.
void trace_duration_push(trace_t* trace, const char* name);

/// End tracing the currently active duration on the current thread.
/// @param trace Trace which will have its active duration popped off.
void trace_duration_pop(trace_t* trace);

/// Start recording trace events.
/// A Chrome trace file will be written to path.
/// @param trace A trace object that will start it's capture of its events.
/// @param path File name where trace capture is recorded
void trace_capture_start(trace_t* trace, const char* path);

/// Stop recording trace events.
/// @param trace Trace object for which capture is to be stopped.
void trace_capture_stop(trace_t* trace);
