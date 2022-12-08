#pragma once


/// Handle to a thread-safe queue.
typedef struct queue_t queue_t;

/// Handle to heap for memory allocation
typedef struct heap_t heap_t;

/// Create a queue with the defined capacity.
/// @param heap Heap for memory allocation of queue.
/// @param capacity Integer for capacity of the queue.
/// @return A pointer to the created queue.
queue_t* queue_create(heap_t* heap, int capacity);

/// Destroy a previously created queue.
/// @param queue Given queue to be destroyed.
void queue_destroy(queue_t* queue);

/// Push an item onto a queue.
/// If the queue is full, blocks until space is available.
/// Safe for multiple threads to push at the same time.
/// @param queue Handle to queue for given item to be pushed into.
/// @param item Pointer to item to be pushed into given queue.
void queue_push(queue_t* queue, void* item);

/// Pop an item off a queue (FIFO order).
/// If the queue is empty, blocks until an item is avaiable.
/// Safe for multiple threads to pop at the same time.
/// @param queue Handle to queue which an item is to be popped off of.
/// @return The first item that was/is put into the queue.
void* queue_pop(queue_t* queue);

/// Pop an item off a queue (FIFO order).
/// If the queue is empty, returns NULL.
/// Safe for multiple threads to pop at the same time.
/// @param queue Handle to queue which an item is to be popped off of.
/// @return The first item that was put into the queue. Null if the queue is empty.
void* queue_try_pop(queue_t* queue);
