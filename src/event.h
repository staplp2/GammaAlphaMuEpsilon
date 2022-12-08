#pragma once

#include <stdbool.h>

/// Handle to an event.
typedef struct event_t event_t;

/// Creates a new event.
event_t* event_create();

/// Destroys a previously created event.
/// @param event Event to be destroyed.
void event_destroy(event_t* event);

/// Signals an event.
/// All threads waiting on this event will resume.
/// @param event Event to be signaled.
void event_signal(event_t* event);

/// Waits for an event to be signaled.
/// @param event Event ehich will be waited on til signaled.
void event_wait(event_t* event);

/// Determines if an event is signaled.
/// @param event Event which will be checked for if it has been signaled.
/// @return True if the event has been signaled. False if the event has yet to be signaled.
bool event_is_raised(event_t* event);
