#pragma once

/// Handle to a mutex.
typedef struct mutex_t mutex_t;

/// Creates a new mutex.
/// @return Handle to created mutex
mutex_t* mutex_create();

/// Destroys a previously created mutex.
/// @param mutex Mutex which is to be destroyed.
void mutex_destroy(mutex_t* mutex);

/// Locks a mutex. May block if another thread unlocks it.
/// If a thread locks a mutex multiple times, it must be unlocked
/// multiple times.
/// @param mutex Mutex which is to be locked.
void mutex_lock(mutex_t* mutex);

/// Unlocks a mutex.
/// @param mutex Mutex which is to be locked.
void mutex_unlock(mutex_t* mutex);
