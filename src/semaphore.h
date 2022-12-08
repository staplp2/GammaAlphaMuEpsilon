#pragma once

#include <stdbool.h>

/// Handle to a semaphore.
typedef struct semaphore_t semaphore_t;

/// Creates a new semaphore.
/// @param initial_count Integer which the sephamore count is set to.
/// @param max_count Integer amount which is te max value a sephamore can be raised to.
/// @return The created semaphore
semaphore_t* semaphore_create(int initial_count, int max_count);

/// Destroys a previously created semaphore.
/// @param semaphore Handle to semaphore which is to be destroyed.
void semaphore_destroy(semaphore_t* semaphore);

/// Lowers the semaphore count by one.
/// If the semaphore count is zero, blocks until another thread releases.
/// @param semaphore Handle to semaphore which is to be lowered.
void semaphore_acquire(semaphore_t* semaphore);

/// Attempts to lower the semaphore count by one.
/// If the semaphore count is zero, returns false. Otherwise true.
/// @param semaphore Handle to semaphore which is to be lowered.
/// @return False if semaphore count is zero. True elsewise.
bool semaphore_try_acquire(semaphore_t* semaphore);

/// Raises the semaphore count by one.
/// @param semaphore Handle to semaphore which is to be raised.
void semaphore_release(semaphore_t* semaphore);
