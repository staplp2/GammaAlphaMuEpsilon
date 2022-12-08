#pragma once

#include <stdint.h>

/// Handle to a thread.
typedef struct thread_t thread_t;

/// Creates a new thread.
/// Thread begins running function with data on return.
/// @param function Function to be run on the given data while in the thread.
/// @param data Parameter for the given function to run with.
/// @return The created thread
thread_t* thread_create(int (*function)(void*), void* data);

/// Waits for a thread to complete and destroys it.
/// @param thread Given thread to destroy.
/// @return The thread's exit code.
int thread_destroy(thread);

/// Puts the calling thread to sleep for the specified number of milliseconds.
/// Thread will sleep for *approximately* the specified time.
/// @param ms Time for thread to sleep.
void thread_sleep(uint32_t ms);
